#include <gtk/gtk.h>
#include "../../pobcurl.c"

#include <string.h> //strcpy()
#include <libgen.h> //basename()

GObject *btStart, *entry ;
gdouble progress;
pobInfo inf;


//Simple messagebox for display errors
void
err_message(GtkWidget *main_window,gchar* title, gchar *msg, gchar* winlabel)
{
	GtkWidget *dialog;
	title=g_strdup_printf("<span size=\"x-large\"><b>%s</b></span>", title);
 
	dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (main_window),
				   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_MESSAGE_ERROR,
				   GTK_BUTTONS_OK,title);
  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
    												"%s",(gchar *)msg);
  gtk_window_set_title(GTK_WINDOW(dialog),winlabel);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}


// This function is started (from glib timeout) every 1 second . 
// It update the progressbar.
gboolean updateProgressbar(gpointer   data)
{
	g_print("UpdateProgressbar...");
	if (progress>0)gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(data), progress);
	progress=inf.percent/100;
	
	
	//Connection Error
	size_t len = strlen(inf.errbuf);
	if (len)
	{
		 g_print("Download error:%s\n",inf.errbuf);
		 err_message(NULL,"Error:", inf.errbuf, "Errors on downloads");
		 pobCurlCleanErr(&inf);
	}
	
	//Http error
	if(inf.http_code>=400)
	{
		gchar* httpErr=g_strdup_printf("%ld",inf.http_code);
		err_message(NULL,"Http Error:", httpErr, "Errors on downloads");
	}
	
	//End download
	if (inf.end==true)
	{
		 gtk_widget_set_sensitive(GTK_WIDGET(btStart), TRUE) ;
		 gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(data), 1.0);
		 g_print("\n Stop updateProgressbar .\n");
		 pobCurlClean(&inf);
		 return FALSE;
	}
	
	
	return TRUE;
}

void dl (GtkWidget *widget, gpointer   data)
{
	//Start Pobcurl
	strcpy(inf.url,gtk_entry_get_text(GTK_ENTRY(entry)));//url from web
	strcpy(inf.dest,basename(inf.url)); //dest filename
	pobCurlStartDownloadThread(&inf);
	
	// Gui stuff
	progress=0;
	g_timeout_add(1000, updateProgressbar, data);//Check progressbar every 1 second
	gtk_widget_set_sensitive(widget, FALSE) ;
}

void on_mainWindow_delete_event(GtkWidget *widget, gpointer   data)
{
	gtk_main_quit();
}

// If I click the stop button the download is canceled
void abortDownload (GtkWidget *widget, gpointer   data)
{
	inf.abort=1;
}

//GUI
void mainWindowInit()
{
	GError* error = NULL;
	gchar* glade_file = g_build_filename("gui.ui", NULL);
	GtkBuilder *xml;  
	GObject *mainWindow, *progressBar,  *lbl, *btStop ;

	xml = gtk_builder_new ();
	if (!gtk_builder_add_from_file  (xml, glade_file, &error))
	{
		g_warning ("Couldn\'t load builder file: %s", error->message);
		g_error_free (error);
	}

	mainWindow=gtk_builder_get_object (xml,"mainWindow" );
	btStart=gtk_builder_get_object (xml,"btStart" );
	btStop=gtk_builder_get_object (xml,"btAbort" );
	progressBar=gtk_builder_get_object(xml,"progressBar");
	entry=gtk_builder_get_object(xml,"entry");
	lbl=gtk_builder_get_object(xml,"lbl");
	
	
	g_object_unref( G_OBJECT( xml ) );
	g_signal_connect (mainWindow, "destroy", G_CALLBACK (on_mainWindow_delete_event), NULL);
	g_signal_connect (btStart, "clicked", G_CALLBACK (dl),progressBar );
	g_signal_connect (btStop, "clicked", G_CALLBACK (abortDownload),NULL);


}




int main (int    argc,  char **argv)
{
	
	gtk_init (&argc, &argv);
	mainWindowInit();
	gtk_main ();

  return 0;
}
