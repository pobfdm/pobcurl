#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "../../pobcurl.c"

#include <string.h> //strcpy()
#include <libgen.h> //basename()

#define MAXDL 20

GObject *btAdd, *entry, *btClear ;
GObject *liststoreDownloads, *treeSelection, *treeviewDownloads;
GtkTreeIter    iter;
pobInfo inf[MAXDL];
gint id=1 ;



enum
	{
	  COL_ID  ,
	  COL_URL ,
	  COL_PROGRESS,
	  COL_STATUS,
	  NUM_COLS
	} ;


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
	
	
	GtkTreeIter  iter;
    gboolean     valid;


    /* Get first row in list store */
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststoreDownloads), &iter);

    while (valid)
    {
       
		int cidx;
		gtk_tree_model_get(GTK_TREE_MODEL(liststoreDownloads), &iter, COL_ID, &cidx, -1);
		
		if(inf[cidx].percentInt>=0 && inf[cidx].percentInt<=100 )
			gtk_list_store_set(GTK_LIST_STORE(liststoreDownloads), &iter, COL_PROGRESS, inf[cidx].percentInt, -1);
	
		//Connection error
		size_t len = strlen(inf[cidx].errbuf);
		if (len)
		{
			 g_print("Download error:%s\n",inf[cidx].errbuf);
			 gtk_list_store_set(GTK_LIST_STORE(liststoreDownloads), &iter, COL_STATUS, inf[cidx].errbuf, -1);
			 inf[cidx].end=true;
		}
		
		
		//Http error
		if(inf[cidx].http_code>=400)
		{
			gchar* httpErr=g_strdup_printf("Error on download file: %ld",inf[cidx].http_code);
			gtk_list_store_set(GTK_LIST_STORE(liststoreDownloads), &iter, COL_STATUS, httpErr, -1);
			inf[cidx].end=true;
			g_remove (inf[cidx].dest);
		}
		
		//End widout errors
		if (inf[cidx].end==true && inf[cidx].http_code<400 && len==0 )
		{
			 //pobCurlClean(&inf[cidx]);
			 gtk_list_store_set(GTK_LIST_STORE(liststoreDownloads), &iter, COL_STATUS, "Success", -1);
		}
	
		
		
		/* Make iter point to the next row in the list store */
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(liststoreDownloads), &iter);
    }
	
	return TRUE;
}


void addDl (GtkWidget *widget, gpointer   data)
{
	
	
	
	gchar* filename=g_path_get_basename (gtk_entry_get_text(GTK_ENTRY(entry)));
	
	gtk_list_store_append (GTK_LIST_STORE(liststoreDownloads), &iter);
	gtk_list_store_set (GTK_LIST_STORE(liststoreDownloads), &iter,
												COL_ID, id,
												COL_URL, filename,
												COL_PROGRESS, 0,	
												-1);
	
	inf[id].url=strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	inf[id].dest=strdup(basename(inf[id].url));
	//inf[id].dest=strdup("video.mp4");
	pobCurlStartDownloadThread(&inf[id]);
											
	
	if (id==1) g_timeout_add(1000, updateProgressbar, NULL);
	id++;

}





void on_mainWindow_delete_event(GtkWidget *widget, gpointer   data)
{
	gtk_main_quit();
}

// If I click the stop button the download is canceled
void abortDownload (GtkWidget *widget, gpointer   data)
{
	int i;
	gtk_tree_model_get(GTK_TREE_MODEL(liststoreDownloads), &iter, COL_ID, &i, -1);
	inf[i].abort=1;
	
}

void clearDls (GtkWidget *widget, gpointer   data)
{
	id=1;
	gtk_list_store_clear (GTK_LIST_STORE(liststoreDownloads));
	int i;
	for (i=0 ; i<=MAXDL;i++) inf[i].abort=1;
	sleep(1);
	for (i=0 ; i<=MAXDL;i++) pobCurlClean(&inf[i]);

}

static void
  onRowActivated (GtkTreeView        *view,
                  GtkTreePath        *path,
                  GtkTreeViewColumn  *col, //non usato
                  gpointer            user_data)
{
 
    
 
    GtkTreeModel *model;
    GtkTreeSelection *sel = gtk_tree_view_get_selection (view);
 
    g_print ("Row selected.\n");
 
    model = gtk_tree_view_get_model(view);
	if (gtk_tree_model_get_iter(model, &iter, path))
	{
		//gint id;
		/*gchar* url;
		gtk_tree_model_get(model, &iter, COL_URL, &url, -1);
		gtk_entry_set_text(GTK_ENTRY(entry),url);
		g_free(url);*/
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
	btClear=gtk_builder_get_object (xml,"btClear" );
	progressBar=gtk_builder_get_object(xml,"progressBar");
	entry=gtk_builder_get_object(xml,"entry");
	lbl=gtk_builder_get_object(xml,"lbl");
	
	treeviewDownloads=gtk_builder_get_object(xml,"treeviewDownloads");
	liststoreDownloads=gtk_builder_get_object(xml,"liststoreDownloads");
	treeSelection=gtk_builder_get_object(xml,"treeSelectionDownloads");
	
	
	
	g_object_unref( G_OBJECT( xml ) );
	g_signal_connect (mainWindow, "destroy", G_CALLBACK (on_mainWindow_delete_event), NULL);
	g_signal_connect (btAdd, "clicked", G_CALLBACK (addDl),NULL );
	g_signal_connect (btClear, "clicked", G_CALLBACK (clearDls),NULL );
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
