#include "dialog.hpp"
#include <string>
#include <iostream>

typedef struct {
    char* id;
    char* label;
} ComboItem;

ComboItem formats[] = {
    {"I4", "I4"},
    {"I8", "I8"},
    {"IA4", "IA4"},
    {"IA8", "IA8"},
    {"RGB565", "RGB565"},
    {"RGB5A3", "RGB5A3"},
    {"CMPR", "CMPR"},
};

void doResponse(GtkWidget* widget, gint responseID, gpointer data){
    *(GtkResponseType*)data = (GtkResponseType)responseID;
    gtk_widget_destroy(widget);
}

void setFormat(GtkWidget *widget, gpointer data){
    gchar* str = gimp_string_combo_box_get_active(GIMP_STRING_COMBO_BOX(widget));
    std::string formatID(str);

    if(formatID == "I4"){
        *((uint8_t*)data) = 0x00;
    } else if(formatID == "I8"){
        *((uint8_t*)data) = 0x01;
    } else if(formatID == "IA4"){
        *((uint8_t*)data) = 0x02;
    } else if(formatID == "IA8"){
        *((uint8_t*)data) = 0x03;
    } else if(formatID == "RGB565"){
        *((uint8_t*)data) = 0x04;
    } else if(formatID == "RGB5A3"){
        *((uint8_t*)data) = 0x05;
    } else if(formatID == "CMPR"){
        *((uint8_t*)data) = 0x0E;
    }
}

GtkResponseType showBtiSaveDialog(uint8_t* format){
    GtkResponseType response;
    GtkWidget* dialog = gimp_export_dialog_new("tex0", "file-tex0", "file-tex-save");

    g_signal_connect(dialog, "response", G_CALLBACK(doResponse), &response);
    g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkListStore* comboList = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    for(int i = 0; i < sizeof(formats) / sizeof(ComboItem); i++){
        gtk_list_store_insert_with_values(comboList, NULL, -1, 0, formats[i].id, 1, formats[i].label, -1);
    }

    GtkWidget* box = gtk_hbox_new(TRUE, 2);
    GtkWidget* fmtLabel = gtk_label_new("Pixel Format:");
    GtkWidget* formatCombo = gimp_string_combo_box_new(GTK_TREE_MODEL(comboList), 0, 1);
    g_object_unref(comboList);

    gtk_box_pack_start(GTK_BOX(box), fmtLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), formatCombo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gimp_export_dialog_get_content_area(dialog)), box, FALSE, FALSE, 2);

    gimp_string_combo_box_set_active(GIMP_STRING_COMBO_BOX(formatCombo), formats[0].id);
    
    gtk_widget_show(fmtLabel);
    gtk_widget_show(formatCombo);
    gtk_widget_show(box);

    g_signal_connect(formatCombo, "changed", G_CALLBACK(setFormat), format);

    gtk_widget_show(dialog);
    gtk_main();

    return response;
}