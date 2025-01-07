#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <string.h>
#include <stdint.h>
#include "dialog.hpp"
#define BSTREAM_IMPLEMENTATION
#include <Bti.hpp>

const char BINARY_NAME[]    = "file-bti";
const char LOAD_PROCEDURE[] = "file-bti-load";
const char SAVE_PROCEDURE[] = "file-bti-save";

int max(int x, int y){
    return x < y ? y : x;
}
int min(int x, int y){
    return x < y ? x : y;
}


void query();
void run(const gchar *, gint, const GimpParam *, gint *, GimpParam **);

GimpPlugInInfo PLUG_IN_INFO = {
    NULL,
    NULL,
    query,
    run
};

MAIN()

void query()
{
    static const GimpParamDef load_arguments[] = {
        { GIMP_PDB_INT32,  "run-mode",     "Interactive, non-interactive" },
        { GIMP_PDB_STRING, "filename",     "The name of the file to load" },
        { GIMP_PDB_STRING, "raw-filename", "The name entered" }
    };

    static const GimpParamDef load_return_values[] = {
        { GIMP_PDB_IMAGE, "image", "Output image" }
    };

    static const GimpParamDef export_arguments[] = {
        { GIMP_PDB_INT32,    "run-mode",      "Interactive, non-interactive" },
        { GIMP_PDB_IMAGE,    "image",         "Input image" },
        { GIMP_PDB_DRAWABLE, "drawable",      "Drawable to save" },
        { GIMP_PDB_STRING,   "filename",      "The name of the file to save the image to" },
        { GIMP_PDB_STRING,   "raw-filename",  "The name entered" }
    };

    gimp_install_procedure(LOAD_PROCEDURE, "Loads BTI Textures", "Loads BTI Textures", "veebs", "Copyright (C) 2025 veebs", "2025", "BTI Texture", NULL, GIMP_PLUGIN, G_N_ELEMENTS(load_arguments), G_N_ELEMENTS(load_return_values), load_arguments, load_return_values);
    gimp_install_procedure(SAVE_PROCEDURE, "Saves BTI Textures", "Saves BTI Textures", "veebs", "Copyright (C) 2025 veebs", "2025", "BTI Texture", "RGB*", GIMP_PLUGIN, G_N_ELEMENTS(export_arguments), 0, export_arguments, NULL);

    gimp_register_load_handler(LOAD_PROCEDURE, "bti", "");
    gimp_register_save_handler(SAVE_PROCEDURE, "bti", "");
}

gboolean load_image(const gchar *filename, gint32 *imageID, GError **error)
{
    gchar                *indata      = NULL;
    gsize                 indatalen;
    gint                  width;
    gint                  height;

    do {
        bStream::CFileStream stream(std::string(filename), bStream::Endianess::Big, bStream::OpenMode::In);

        Bti img;
        if(img.Load(&stream)){
            *imageID = gimp_image_new(img.mWidth, img.mHeight, GIMP_RGB);
            GdkPixbuf* imgBuff = gdk_pixbuf_new_from_data(img.GetData(), GDK_COLORSPACE_RGB, TRUE, 8, img.mWidth, img.mHeight, img.mWidth*4, NULL, NULL);
            gint32 layerID = gimp_layer_new_from_pixbuf(*imageID, "background", imgBuff, 100, GIMP_LAYER_MODE_NORMAL, 0.0, 0.0);
            gimp_image_insert_layer(*imageID, layerID, -1, 0);
            g_object_unref(imgBuff);
        }
    } while(0);

    return TRUE;
}

/* This function is called when one of our methods is invoked. */
void run(const gchar * name, gint nparams, const GimpParam * param, gint * nreturn_vals, GimpParam ** return_vals)
{
    static GimpParam values[2];
    GimpRunMode runMode;
    GimpPDBStatusType status = GIMP_PDB_SUCCESS;
    gint32 imageID;
    gint32 drawableID;
    GError* error = NULL;

    /* Determine the current run mode */
    runMode = (GimpRunMode)param[0].data.d_int32;

    /* Fill in the return values */
    *nreturn_vals = 1;
    *return_vals  = values;
    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;

    if(!strcmp(name, LOAD_PROCEDURE)) {
        if(load_image(param[1].data.d_string, &imageID, &error) == TRUE) {

            /* Return the new image that was loaded */
            *nreturn_vals = 2;
            values[1].type = GIMP_PDB_IMAGE;
            values[1].data.d_image = imageID;

        } else {
            status = GIMP_PDB_EXECUTION_ERROR;
        }

    } else if(!strcmp(name, SAVE_PROCEDURE)) {
        GimpExportReturn export_ret = GIMP_EXPORT_CANCEL;

        imageID = param[1].data.d_int32;
        drawableID = param[2].data.d_int32;
        
        Bti toSave;

        switch(runMode) {
        case GIMP_RUN_INTERACTIVE:
        case GIMP_RUN_WITH_LAST_VALS:
            gimp_ui_init(BINARY_NAME, FALSE);
            export_ret = gimp_export_image(&imageID, &drawableID, "bti", (GimpExportCapabilities)(GIMP_EXPORT_CAN_HANDLE_RGB | GIMP_EXPORT_CAN_HANDLE_ALPHA));

            if(export_ret == GIMP_EXPORT_CANCEL) {
                values[0].data.d_status = GIMP_PDB_CANCEL;
                return;
            }
            

            uint8_t imgFormat;
            if(showBtiSaveDialog(&imgFormat) != GTK_RESPONSE_OK) {
                values[0].data.d_status = GIMP_PDB_CANCEL;
                break;
            }

            toSave.SetFormat(imgFormat);
            break;

        case GIMP_RUN_NONINTERACTIVE:
            if(nparams != 11) {
                status = GIMP_PDB_CALLING_ERROR;
                break;
            }


            break;
        }

        gint bpp = gimp_drawable_bpp(drawableID);
        gint width = gimp_drawable_width(drawableID);
        gint height = gimp_drawable_height(drawableID);
        GimpImageType drawable_type = gimp_drawable_type(drawableID);

        std::vector<guchar> imgBuffer(bpp * width * height);

#ifdef GIMP_2_9
        GeglBuffer* buffer = gimp_drawable_get_buffer(drawableID);
        GeglRectangle extent = *gegl_buffer_get_extent(geglbuffer);
        gegl_buffer_get(buffer, &extent, 1.0, NULL, imgBuffer.data(), GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
        g_object_unref(buffer);
#else
        GimpPixelRgn region;
        GimpDrawable* drawable = NULL;
        drawable = gimp_drawable_get(drawableID);
        gimp_pixel_rgn_init(&region, drawable, 0, 0, width, height, FALSE, FALSE);
        gimp_pixel_rgn_get_rect(&region, imgBuffer.data(), 0, 0, width, height);
        gimp_drawable_detach(drawable);
#endif
        std::cout << width << " " << height << std::endl;
        bStream::CFileStream file(std::string(param[3].data.d_string), bStream::Endianess::Big, bStream::OpenMode::Out);

        toSave.SetData(width, height, imgBuffer.data());
        toSave.Save(&file);
        status = GimpPDBStatusType::GIMP_PDB_SUCCESS;

        std::cout << "wrote image!" << std::endl;
    }

    if(status != GIMP_PDB_SUCCESS && error) {
        *nreturn_vals = 2;
        values[1].type = GIMP_PDB_STRING;
        values[1].data.d_string = error->message;
    }

    values[0].data.d_status = status;
}