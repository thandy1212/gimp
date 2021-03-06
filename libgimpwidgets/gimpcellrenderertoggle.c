/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * gimpcellrenderertoggle.c
 * Copyright (C) 2003-2004  Sven Neumann <sven@gimp.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <gtk/gtk.h>

#include "gimpwidgetstypes.h"

#include "gimpwidgetsmarshal.h"
#include "gimpcellrenderertoggle.h"


/**
 * SECTION: gimpcellrenderertoggle
 * @title: GimpCellRendererToggle
 * @short_description: A #GtkCellRendererToggle that displays icons instead
 *                     of a checkbox.
 *
 * A #GtkCellRendererToggle that displays icons instead of a checkbox.
 **/


#define DEFAULT_ICON_SIZE  GTK_ICON_SIZE_BUTTON


enum
{
  CLICKED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_ICON_NAME,
  PROP_STOCK_ID,
  PROP_STOCK_SIZE,
  PROP_OVERRIDE_BACKGROUND
};


typedef struct _GimpCellRendererTogglePrivate GimpCellRendererTogglePrivate;

struct _GimpCellRendererTogglePrivate
{
  gchar    *icon_name;
  gboolean  override_background;
};

#define GET_PRIVATE(obj) \
        G_TYPE_INSTANCE_GET_PRIVATE (obj, \
                                     GIMP_TYPE_CELL_RENDERER_TOGGLE, \
                                     GimpCellRendererTogglePrivate)


static void gimp_cell_renderer_toggle_finalize     (GObject         *object);
static void gimp_cell_renderer_toggle_get_property (GObject         *object,
                                                    guint            param_id,
                                                    GValue          *value,
                                                    GParamSpec      *pspec);
static void gimp_cell_renderer_toggle_set_property (GObject         *object,
                                                    guint            param_id,
                                                    const GValue    *value,
                                                    GParamSpec      *pspec);
static void gimp_cell_renderer_toggle_get_size     (GtkCellRenderer *cell,
                                                    GtkWidget       *widget,
                                                    GdkRectangle    *rectangle,
                                                    gint            *x_offset,
                                                    gint            *y_offset,
                                                    gint            *width,
                                                    gint            *height);
static void gimp_cell_renderer_toggle_render       (GtkCellRenderer *cell,
                                                    GdkWindow       *window,
                                                    GtkWidget       *widget,
                                                    GdkRectangle    *background_area,
                                                    GdkRectangle    *cell_area,
                                                    GdkRectangle    *expose_area,
                                                    GtkCellRendererState flags);
static gboolean gimp_cell_renderer_toggle_activate (GtkCellRenderer *cell,
                                                    GdkEvent        *event,
                                                    GtkWidget       *widget,
                                                    const gchar     *path,
                                                    GdkRectangle    *background_area,
                                                    GdkRectangle    *cell_area,
                                                    GtkCellRendererState  flags);
static void gimp_cell_renderer_toggle_create_pixbuf (GimpCellRendererToggle *toggle,
                                                     GtkWidget              *widget);


G_DEFINE_TYPE (GimpCellRendererToggle, gimp_cell_renderer_toggle,
               GTK_TYPE_CELL_RENDERER_TOGGLE)

#define parent_class gimp_cell_renderer_toggle_parent_class

static guint toggle_cell_signals[LAST_SIGNAL] = { 0 };


static void
gimp_cell_renderer_toggle_class_init (GimpCellRendererToggleClass *klass)
{
  GObjectClass         *object_class = G_OBJECT_CLASS (klass);
  GtkCellRendererClass *cell_class   = GTK_CELL_RENDERER_CLASS (klass);

  toggle_cell_signals[CLICKED] =
    g_signal_new ("clicked",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GimpCellRendererToggleClass, clicked),
                  NULL, NULL,
                  _gimp_widgets_marshal_VOID__STRING_FLAGS,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  GDK_TYPE_MODIFIER_TYPE);

  object_class->finalize     = gimp_cell_renderer_toggle_finalize;
  object_class->get_property = gimp_cell_renderer_toggle_get_property;
  object_class->set_property = gimp_cell_renderer_toggle_set_property;

  cell_class->get_size       = gimp_cell_renderer_toggle_get_size;
  cell_class->render         = gimp_cell_renderer_toggle_render;
  cell_class->activate       = gimp_cell_renderer_toggle_activate;

  g_object_class_install_property (object_class, PROP_ICON_NAME,
                                   g_param_spec_string ("icon-name",
                                                        "Icon Name",
                                                        "The icon to display",
                                                        NULL,
                                                        GIMP_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_STOCK_ID,
                                   g_param_spec_string ("stock-id",
                                                        "Stock ID",
                                                        "The icon to display, deprecated",
                                                        NULL,
                                                        GIMP_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_STOCK_SIZE,
                                   g_param_spec_int ("stock-size",
                                                     "Stock Size",
                                                     "The icon size to use",
                                                     0, G_MAXINT,
                                                     DEFAULT_ICON_SIZE,
                                                     GIMP_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_OVERRIDE_BACKGROUND,
                                   g_param_spec_boolean ("override-background",
                                                         "Override Background",
                                                         "Draw the background if the row is selected",
                                                         FALSE,
                                                         GIMP_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));

  g_type_class_add_private (object_class, sizeof (GimpCellRendererTogglePrivate));
}

static void
gimp_cell_renderer_toggle_init (GimpCellRendererToggle *toggle)
{
}

static void
gimp_cell_renderer_toggle_finalize (GObject *object)
{
  GimpCellRendererToggle        *toggle = GIMP_CELL_RENDERER_TOGGLE (object);
  GimpCellRendererTogglePrivate *priv   = GET_PRIVATE (object);

  g_clear_pointer (&priv->icon_name,  g_free);
  g_clear_pointer (&toggle->stock_id, g_free);

  g_clear_object (&toggle->pixbuf);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_cell_renderer_toggle_get_property (GObject    *object,
                                        guint       param_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  GimpCellRendererToggle        *toggle = GIMP_CELL_RENDERER_TOGGLE (object);
  GimpCellRendererTogglePrivate *priv   = GET_PRIVATE (object);

  switch (param_id)
    {
    case PROP_ICON_NAME:
      g_value_set_string (value, priv->icon_name);
      break;

    case PROP_STOCK_ID:
      g_value_set_string (value, toggle->stock_id);
      break;

    case PROP_STOCK_SIZE:
      g_value_set_int (value, toggle->stock_size);
      break;

    case PROP_OVERRIDE_BACKGROUND:
      g_value_set_boolean (value, priv->override_background);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
gimp_cell_renderer_toggle_set_property (GObject      *object,
                                        guint         param_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  GimpCellRendererToggle        *toggle = GIMP_CELL_RENDERER_TOGGLE (object);
  GimpCellRendererTogglePrivate *priv   = GET_PRIVATE (object);

  switch (param_id)
    {
    case PROP_ICON_NAME:
      if (priv->icon_name)
        g_free (priv->icon_name);
      priv->icon_name = g_value_dup_string (value);
      break;

    case PROP_STOCK_ID:
      if (toggle->stock_id)
        g_free (toggle->stock_id);
      toggle->stock_id = g_value_dup_string (value);
      break;

    case PROP_STOCK_SIZE:
      toggle->stock_size = g_value_get_int (value);
      break;

    case PROP_OVERRIDE_BACKGROUND:
      priv->override_background = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }

  g_clear_object (&toggle->pixbuf);
}

static void
gimp_cell_renderer_toggle_get_size (GtkCellRenderer *cell,
                                    GtkWidget       *widget,
                                    GdkRectangle    *cell_area,
                                    gint            *x_offset,
                                    gint            *y_offset,
                                    gint            *width,
                                    gint            *height)
{
  GimpCellRendererToggle        *toggle = GIMP_CELL_RENDERER_TOGGLE (cell);
  GimpCellRendererTogglePrivate *priv   = GET_PRIVATE (cell);
  GtkStyle                      *style  = gtk_widget_get_style (widget);
  gint                           calc_width;
  gint                           calc_height;
  gint                           pixbuf_width;
  gint                           pixbuf_height;
  gfloat                         xalign;
  gfloat                         yalign;
  gint                           xpad;
  gint                           ypad;

  if (! priv->icon_name && ! toggle->stock_id)
    {
      GTK_CELL_RENDERER_CLASS (parent_class)->get_size (cell,
                                                        widget,
                                                        cell_area,
                                                        x_offset, y_offset,
                                                        width, height);
      return;
    }

  gtk_cell_renderer_get_alignment (cell, &xalign, &yalign);
  gtk_cell_renderer_get_padding (cell, &xpad, &ypad);

  if (! toggle->pixbuf)
    gimp_cell_renderer_toggle_create_pixbuf (toggle, widget);

  pixbuf_width  = gdk_pixbuf_get_width  (toggle->pixbuf);
  pixbuf_height = gdk_pixbuf_get_height (toggle->pixbuf);

  calc_width  = (pixbuf_width +
                 (gint) xpad * 2 + style->xthickness * 2);
  calc_height = (pixbuf_height +
                 (gint) ypad * 2 + style->ythickness * 2);

  if (width)
    *width  = calc_width;

  if (height)
    *height = calc_height;

  if (cell_area)
    {
      if (x_offset)
        {
          *x_offset = (((gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) ?
                        (1.0 - xalign) : xalign) *
                       (cell_area->width - calc_width));
          *x_offset = MAX (*x_offset, 0);
        }

      if (y_offset)
        {
          *y_offset = yalign * (cell_area->height - calc_height);
          *y_offset = MAX (*y_offset, 0);
        }
    }
}

static void
gimp_cell_renderer_toggle_render (GtkCellRenderer      *cell,
                                  GdkWindow            *window,
                                  GtkWidget            *widget,
                                  GdkRectangle         *background_area,
                                  GdkRectangle         *cell_area,
                                  GdkRectangle         *expose_area,
                                  GtkCellRendererState  flags)
{
  GimpCellRendererToggle        *toggle = GIMP_CELL_RENDERER_TOGGLE (cell);
  GimpCellRendererTogglePrivate *priv   = GET_PRIVATE (cell);
  GtkStyle                      *style  = gtk_widget_get_style (widget);
  GdkRectangle                   toggle_rect;
  GdkRectangle                   draw_rect;
  GtkStateType                   state;
  gboolean                       active;
  gint                           xpad;
  gint                           ypad;

  if (! priv->icon_name && ! toggle->stock_id)
    {
      GTK_CELL_RENDERER_CLASS (parent_class)->render (cell, window, widget,
                                                      background_area,
                                                      cell_area, expose_area,
                                                      flags);
      return;
    }

  if ((flags & GTK_CELL_RENDERER_SELECTED) &&
      priv->override_background)
    {
      gboolean background_set;

      g_object_get (cell,
                    "cell-background-set", &background_set,
                    NULL);

      if (background_set)
        {
          cairo_t  *cr = gdk_cairo_create (window);
          GdkColor *color;

          g_object_get (cell,
                        "cell-background-gdk", &color,
                        NULL);

          gdk_cairo_rectangle (cr, background_area);
          gdk_cairo_set_source_color (cr, color);
          cairo_fill (cr);

          gdk_color_free (color);
          cairo_destroy (cr);
        }
    }

  gimp_cell_renderer_toggle_get_size (cell, widget, cell_area,
                                      &toggle_rect.x,
                                      &toggle_rect.y,
                                      &toggle_rect.width,
                                      &toggle_rect.height);

  gtk_cell_renderer_get_padding (cell, &xpad, &ypad);

  toggle_rect.x      += cell_area->x + xpad;
  toggle_rect.y      += cell_area->y + ypad;
  toggle_rect.width  -= xpad * 2;
  toggle_rect.height -= ypad * 2;

  if (toggle_rect.width <= 0 || toggle_rect.height <= 0)
    return;

  active =
    gtk_cell_renderer_toggle_get_active (GTK_CELL_RENDERER_TOGGLE (cell));

  if ((flags & GTK_CELL_RENDERER_SELECTED) == GTK_CELL_RENDERER_SELECTED)
    {
      if (gtk_widget_has_focus (widget))
        state = GTK_STATE_SELECTED;
      else
        state = GTK_STATE_ACTIVE;
    }
  else
    {
      if (gtk_cell_renderer_toggle_get_activatable (GTK_CELL_RENDERER_TOGGLE (cell)))
        state = GTK_STATE_NORMAL;
      else
        state = GTK_STATE_INSENSITIVE;
    }

  if (gdk_rectangle_intersect (expose_area, cell_area, &draw_rect) &&
      (flags & GTK_CELL_RENDERER_PRELIT))
    gtk_paint_shadow (style,
                      window,
                      state,
                      active ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
                      &draw_rect,
                      widget, NULL,
                      toggle_rect.x,     toggle_rect.y,
                      toggle_rect.width, toggle_rect.height);

  if (active)
    {
      toggle_rect.x      += style->xthickness;
      toggle_rect.y      += style->ythickness;
      toggle_rect.width  -= style->xthickness * 2;
      toggle_rect.height -= style->ythickness * 2;

      if (gdk_rectangle_intersect (&draw_rect, &toggle_rect, &draw_rect))
        {
          cairo_t  *cr = gdk_cairo_create (window);
          gboolean  inconsistent;

          gdk_cairo_rectangle (cr, &draw_rect);
          cairo_clip (cr);

          gdk_cairo_set_source_pixbuf (cr, toggle->pixbuf,
                                       toggle_rect.x, toggle_rect.y);
          cairo_paint (cr);

          g_object_get (toggle,
                        "inconsistent", &inconsistent,
                        NULL);

          if (inconsistent)
            {
              gdk_cairo_set_source_color (cr, &style->fg[state]);
              cairo_set_line_width (cr, 1.5);
              cairo_move_to (cr,
                             toggle_rect.x + toggle_rect.width - 1,
                             toggle_rect.y + 1);
              cairo_line_to (cr,
                             toggle_rect.x + 1,
                             toggle_rect.y + toggle_rect.height - 1);
              cairo_stroke (cr);
            }

          cairo_destroy (cr);
        }
    }
}

static gboolean
gimp_cell_renderer_toggle_activate (GtkCellRenderer      *cell,
                                    GdkEvent             *event,
                                    GtkWidget            *widget,
                                    const gchar          *path,
                                    GdkRectangle         *background_area,
                                    GdkRectangle         *cell_area,
                                    GtkCellRendererState  flags)
{
  GtkCellRendererToggle *toggle = GTK_CELL_RENDERER_TOGGLE (cell);

  if (gtk_cell_renderer_toggle_get_activatable (toggle))
    {
      GdkModifierType state = 0;

      if (event && ((GdkEventAny *) event)->type == GDK_BUTTON_PRESS)
        state = ((GdkEventButton *) event)->state;

      gimp_cell_renderer_toggle_clicked (GIMP_CELL_RENDERER_TOGGLE (cell),
                                         path, state);

      return TRUE;
    }

  return FALSE;
}

static void
gimp_cell_renderer_toggle_create_pixbuf (GimpCellRendererToggle *toggle,
                                         GtkWidget              *widget)
{
  GimpCellRendererTogglePrivate *priv = GET_PRIVATE (toggle);

  g_clear_object (&toggle->pixbuf);

  if (priv->icon_name)
    {
      gint width, height;

      if (! gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (widget),
                                               toggle->stock_size,
                                               &width, &height))
        {
          width  = 20;
          height = 20;
        }

      toggle->pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                                 priv->icon_name,
                                                 MIN (width, height), 0, NULL);
    }
  else
    {
      toggle->pixbuf = gtk_widget_render_icon (widget,
                                               toggle->stock_id,
                                               toggle->stock_size, NULL);
    }
}


/**
 * gimp_cell_renderer_toggle_new:
 * @icon_name: the icon name of the icon to use for the active state
 *
 * Creates a custom version of the #GtkCellRendererToggle. Instead of
 * showing the standard toggle button, it shows a named icon if the
 * cell is active and no icon otherwise. This cell renderer is for
 * example used in the Layers treeview to indicate and control the
 * layer's visibility by showing %GIMP_STOCK_VISIBLE.
 *
 * Return value: a new #GimpCellRendererToggle
 *
 * Since: 2.2
 **/
GtkCellRenderer *
gimp_cell_renderer_toggle_new (const gchar *icon_name)
{
  return g_object_new (GIMP_TYPE_CELL_RENDERER_TOGGLE,
                       "icon-name", icon_name,
                       NULL);
}

/**
 * gimp_cell_renderer_toggle_clicked:
 * @cell:  a #GimpCellRendererToggle
 * @path:  the path to the clicked row
 * @state: the modifier state
 *
 * Emits the "clicked" signal from a #GimpCellRendererToggle.
 *
 * Since: 2.2
 **/
void
gimp_cell_renderer_toggle_clicked (GimpCellRendererToggle *cell,
                                   const gchar            *path,
                                   GdkModifierType         state)
{
  g_return_if_fail (GIMP_IS_CELL_RENDERER_TOGGLE (cell));
  g_return_if_fail (path != NULL);

  g_signal_emit (cell, toggle_cell_signals[CLICKED], 0, path, state);
}
