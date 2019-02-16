#include <gtk/gtk.h>

GtkWidget *g_window;

/**
 * Fullscreen mode
 */
void on_button_fullscreen_clicked()
{
  gtk_container_set_border_width(GTK_CONTAINER(g_window), 0);
  gtk_widget_realize(g_window);
  gtk_window_fullscreen((GtkWindow *)g_window);
}

/**
 * Exit
 */
void on_button_exit_clicked()
{
  gtk_main_quit();
}

void on_window_main_destroy()
{
  gtk_main_quit();
}


int main(int argc, char *argv[])
{
  GtkBuilder *builder;

  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "data/ui/window.glade", NULL);

  g_window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  gtk_builder_connect_signals(builder, NULL);

  g_object_unref(builder);

  gtk_widget_show(g_window);
  gtk_main();

  return 0;
}
