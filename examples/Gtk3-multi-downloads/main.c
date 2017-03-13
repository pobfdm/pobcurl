#include <gtk/gtk.h>
#include "../../pobcurl.c"

#include <string.h> //strcpy()
#include <libgen.h> //basename()

#define MAXDL 20

GObject *btAdd, *entry ;
GObject *liststoreDownloads, *treeSelection, *treeviewDownloads;
GtkTreeIter    iter;
pobInfo inf[MAXDL];
gint id=1 ;




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
	
	enum
	{
	  COL_ID  ,
	  COL_URL ,
	  COL_PROGRESS,
	  NUM_COLS
	} ;
	GtkTreeIter  iter;
    gboolean     valid;


    /* Get first row in list store */
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststoreDownloads), &iter);

    while (valid)
    {
       
		int cidx;
		gtk_tree_model_get(GTK_TREE_MODEL(liststoreDownloads), &iter, COL_ID, &cidx, -1);
		if(inf[cidx].percentInt>0 && inf[cidx].percentInt<=100)
			gtk_list_store_set(GTK_LIST_STORE(liststoreDownloads), &iter, COL_PROGRESS, inf[cidx].percentInt, -1);

		/* Make iter point to the next row in the list store */
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(liststoreDownloads), &iter);
    }
	
	
	
	
	/*if (progress>0)gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(data), progress);
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
		 gtk_widget_set_sensitive(GTK_WIDGET(btAdd), TRUE) ;
		 gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(data), 1.0);
		 g_print("\n Stop updateProgressbar .\n");
		 pobCurlClean(&inf);
		 return FALSE;
	}*/
	
	
	return TRUE;
}


void addDl (GtkWidget *widget, gpointer   data)
{
	
	enum
	{
	  COL_ID  ,
	  COL_URL ,
	  COL_PROGRESS,
	  NUM_COLS
	} ;
	
	gchar* filename=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	
	gtk_list_store_append (GTK_LIST_STORE(liststoreDownloads), &iter);
	gtk_list_store_set (GTK_LIST_STORE(liststoreDownloads), &iter,
												COL_ID, id,
												//COL_URL, gtk_entry_get_text(GTK_ENTRY(entry)),
												COL_URL, filename,
												COL_PROGRESS, 0,	
												-1);
	
	inf[id].url=strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	inf[id].dest=strdup(basename(inf[id].url));
	pobCurlStartDownloadThread(&inf[id]);
											
	
	g_timeout_add(1000, updateProgressbar, NULL);
	id++;

}





void on_mainWindow_delete_event(GtkWidget *widget, gpointer   data)
{
	gtk_main_quit();
}

// If I click the stop button the download is canceled
void abortDownload (GtkWidget *widget, gpointer   data)
{
	//inf.abort=1;
}



static void
  onRowActivated (GtkTreeView        *view,
                  GtkTreePath        *path,
                  GtkTreeViewColumn  *col, //non usato
                  gpointer            user_data)
{
 
    enum
	{
	  COL_ID  ,
	  COL_URL ,
	  COL_PROGRESS,
	  NUM_COLS
	} ;
 
    GtkTreeModel *model;
    GtkTreeSelection *sel = gtk_tree_view_get_selection (view);
 
    g_print ("Row selected.\n");
 
    model = gtk_tree_view_get_model(view);
	if (gtk_tree_model_get_iter(model, &iter, path))
	{
		//gint id;
		gchar* url;
		gtk_tree_model_get(model, &iter, COL_URL, &url, -1);
		gtk_entry_set_text(GTK_ENTRY(entry),url);
		g_free(url);
	}else{
		//Se non ho selezionato nessun record esco senza far nulla 
		return; 
	}
 
 
 
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
	btAdd=gtk_builder_get_object (xml,"btAdd" );
	btStop=gtk_builder_get_object (xml,"btAbort" );
	progressBar=gtk_builder_get_object(xml,"progressBar");
	entry=gtk_builder_get_object(xml,"entry");
	lbl=gtk_builder_get_object(xml,"lbl");
	
	treeviewDownloads=gtk_builder_get_object(xml,"treeviewDownloads");
	liststoreDownloads=gtk_builder_get_object(xml,"liststoreDownloads");
	treeSelection=gtk_builder_get_object(xml,"treeSelectionDownloads");
	
	
	
	g_object_unref( G_OBJECT( xml ) );
	g_signal_connect (mainWindow, "destroy", G_CALLBACK (on_mainWindow_delete_event), NULL);
	g_signal_connect (btAdd, "clicked", G_CALLBACK (addDl),NULL );
	g_signal_connect (btStop, "clicked", G_CALLBACK (abortDownload),NULL);
	g_signal_connect(treeviewDownloads, "row-activated", G_CALLBACK(onRowActivated), NULL);


}




int main (int    argc,  char **argv)
{
	
	gtk_init (&argc, &argv);
	mainWindowInit();
	gtk_main ();

  return 0;
}
