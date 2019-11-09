#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <stdlib.h>
#include <strings.h>

static GtkWidget *g_window = NULL;
static GtkWidget *g_stars_drawing_area = NULL;
static cairo_surface_t *g_surface = NULL;
static GtkWidget *g_webkit = NULL;

typedef struct
{
  gdouble x;
  gdouble y;
} tStar;

tStar g_stars[200];

static unsigned int g_max_stars = sizeof(g_stars) / sizeof(tStar);
static unsigned int g_max_x = 0;
static unsigned int g_max_y = 0;

/**
 * Repaint the screen from the surface
 * @return FALSE to propagate the event further, TRUE in case of errors
 */
static gboolean on_stars_drawing_area_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  if (g_surface == NULL)
  {
    return TRUE;
  }

  cairo_set_source_surface(cr, g_surface, 0, 0);
  cairo_paint(cr);
  return FALSE;
}

/*
 * Return a valid random position
 */
static gdouble get_random_pos(unsigned int max)
{
  gdouble pos = 0.0;
  do
  {
    pos = ((gdouble)rand() / (gdouble)(RAND_MAX)) * max;
  } while ((pos < 0.0) || (pos > max));
  return pos;
}

/*
 * Randomize stars
 */
static void randomize_stars()
{
  for (int i = 0; i < g_max_stars; i++)
  {
    g_stars[i].x = get_random_pos(g_max_x);
    g_stars[i].y = get_random_pos(g_max_y);
  }
}

/**
 * Store the current width and heigth of the surface
 */
void on_surface_resize(GtkWidget *widget, GtkAllocation *allocation, void *data)
{
  printf("width = %d, height = %d\n", allocation->width, allocation->height);
  g_max_x = allocation->width;
  g_max_y = allocation->height;

  randomize_stars();
}

/**
 * Clear background
 */
static void clear_background(void)
{
  cairo_t *cr = cairo_create(g_surface);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);
}

/**
 * Initialize surface
 */
static gboolean on_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  if (g_surface)
  {
    cairo_surface_destroy(g_surface);
  }

  g_surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
                                                CAIRO_CONTENT_COLOR,
                                                gtk_widget_get_allocated_width(widget),
                                                gtk_widget_get_allocated_height(widget));

  clear_background();

  // configure event handled, return TRUE to avoid further processing
  return TRUE;
}

/**
 * Move the stars around
 */
static gboolean move_the_stars_timer(GtkWidget *widget)
{
  //printf("timer..\n");

  clear_background();

  for (int i = 0; i < g_max_stars; i++)
  {
    if (g_stars_drawing_area != 0)
    {
      cairo_t *cr = cairo_create(g_surface);
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_rectangle(cr, g_stars[i].x - 4, g_stars[i].y - 4, 8, 8);
      cairo_fill(cr);
      cairo_destroy(cr);

      if (!(GTK_IS_WIDGET(widget))) {
        // widget becomes invalid on shutdown
        return FALSE;
      }

      // invalidate the drawing area
      gtk_widget_queue_draw_area(widget, g_stars[i].x - 4, g_stars[i].y - 4, 8, 8);

      g_stars[i].x += (1.0 + (2.0 * (g_stars[i].y / g_max_y)));
      gdouble color = g_stars[i].y / g_max_y;

      if (g_stars[i].x > g_max_x)
      {
        g_stars[i].x = get_random_pos(g_max_x / 5);
        color = 0.3;
      }
      if (g_stars[i].y > g_max_y)
      {
        g_stars[i].y = get_random_pos(g_max_y);
        color = 0.3;
      }

      cr = cairo_create(g_surface);
      cairo_set_source_rgb(cr, color, color, color);
      cairo_rectangle(cr, g_stars[i].x - 3, g_stars[i].y - 3, 6, 6);
      cairo_fill(cr);
      cairo_destroy(cr);
      // invalidate the drawing area
      gtk_widget_queue_draw_area(widget, g_stars[i].x - 3, g_stars[i].y - 3, 6, 6);
    }
  }

  return TRUE;
}

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
  printf("exit button clicked\n");
  gtk_main_quit();
}

void on_window_main_destroy()
{
  printf("close main window\n");
  gtk_main_quit();
}

static gboolean on_key_esc_pressed(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  if (event->keyval == GDK_KEY_Escape)
  {
    printf("ESC key pressed\n");
    gtk_main_quit();
    return TRUE;
  }
  return FALSE;
}

/**
 * Debug
 */
void dump_widgets(GtkWidget *parent)
{
  if (GTK_IS_CONTAINER(parent)) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    while (children) {
        GtkWidget *child = GTK_WIDGET (children->data);
        printf ("  type = %s\n", G_OBJECT_TYPE_NAME (child));

        if (GTK_IS_CONTAINER(child)) {
          dump_widgets(child);
        }
        children = children->next;
    }
  }
}

/**
 * Web
 */
void switch_page(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data)
{
  printf("page_num = %d  - type: %s\n", page_num, G_OBJECT_TYPE_NAME(page));
  if (page_num == 1) {
    const gchar* uri = "http://html5test.com/";
    printf("reloading %s\n", uri);
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(g_webkit), uri);
  }
  dump_widgets(page);
}

/************
 *** MAIN ***
 ************/
int main(int argc, char *argv[])
{
  GError *error = NULL;

  gtk_init(&argc, &argv);

  // Create a browser instance before the GTK builder can run
  WebKitWebView *webkit_init = WEBKIT_WEB_VIEW(webkit_web_view_new());

  // Now build the UI (including a WebKitWebView)
  GtkBuilder *builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "data/ui/window.glade", &error);
  if(error != NULL)
  {
    printf("ERROR: %s\n", error->message);
    g_error_free(error);
    exit(EXIT_FAILURE);
  }

  g_webkit = GTK_WIDGET(gtk_builder_get_object(builder, "webkit"));
  if (g_webkit == 0)
  {
    printf("ERROR: can not find widget 'webkit'\n");
    exit(EXIT_FAILURE);
  }

  g_stars_drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "starsDrawingArea"));
  if (g_stars_drawing_area == 0)
  {
    printf("ERROR: can not find widget 'starsDrawingArea'\n");
    exit(EXIT_FAILURE);
  }

  g_window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  if (g_window == 0)
  {
    printf("ERROR: can not find widget 'window'\n");
    exit(EXIT_FAILURE);
  }

  gtk_builder_connect_signals(builder, NULL);
  g_signal_connect(g_window, "key_press_event", G_CALLBACK(on_key_esc_pressed), NULL);

  g_signal_connect(g_stars_drawing_area, "draw", G_CALLBACK(on_stars_drawing_area_draw), NULL);
  g_signal_connect(g_stars_drawing_area, "size-allocate", G_CALLBACK(on_surface_resize), NULL);
  g_signal_connect(g_stars_drawing_area, "configure-event", G_CALLBACK(on_configure_event), NULL);

  g_timeout_add(20, (GSourceFunc)move_the_stars_timer, (gpointer)g_stars_drawing_area);

  g_object_unref(builder);

  gtk_widget_show(g_window);
  gtk_main();

  return 0;
}
