#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#define RATPOISON "/usr/bin/ratpoison"

typedef struct {
  int x;
  int y;
  int width;
  int height;
} Frame;

static int
get_pointer_coords (int *x, int *y)
{
  Display *display;
  XEvent event;
  int screen;

  if (!(display = XOpenDisplay (NULL)))
    return 0;

  screen = DefaultScreen (display);
  XQueryPointer (display, RootWindow (display, screen),
                 &event.xbutton.root, &event.xbutton.window,
                 &event.xbutton.x_root, &event.xbutton.y_root,
                 &event.xbutton.x, &event.xbutton.y,
                 &event.xbutton.state);
  *x = event.xbutton.x;
  *y = event.xbutton.y;
  return 1;
}

static Frame **
get_frames (int *frame_count)
{
  FILE *stream;
  char output[4096];
  size_t total, count = 0;
  char *line, *content;
  char *saveptr1, *saveptr2;
  Frame **frames = NULL;

  stream = popen (RATPOISON " -c fdump", "r");
  total = fread (output, 1, 4096, stream);
  output[total] = '\0';
  content = (char *) output;

  while ((line = (char *) strtok_r (content, ",", &saveptr1)) != NULL)
    {
      Frame *frame, *tmp;
      char *piece;

      content = NULL;
      if (strlen (line) == 0 || line[0] == '\n')
        break;
      if (line[0] == '(')
        *line++;

      /* Getting rid of the `frame' piece */
      strtok_r (line, " ", &saveptr2);

      /* Filling the frame struct */
      frame = malloc (sizeof (Frame));
      while ((piece = strtok_r (NULL, " ", &saveptr2)) != NULL)
        {
          if (strcmp (piece, ":x") == 0)
            frame->x = atoi (strtok_r (NULL, " ", &saveptr2));
          else if (strcmp (piece, ":y") == 0)
            frame->y = atoi (strtok_r (NULL, " ", &saveptr2));
          else if (strcmp (piece, ":width") == 0)
            frame->width = atoi (strtok_r (NULL, " ", &saveptr2));
          else if (strcmp (piece, ":height") == 0)
            frame->height = atoi (strtok_r (NULL, " ", &saveptr2));
        }

      /* Appending the current frame to the frame list */
      if ((tmp = realloc (frames, sizeof (Frame) * (++count))) == NULL)
        {
          size_t i;
          for (i = 0; i < count-1; i++)
            free (frames[i]);
          free (frames);
          return NULL;
        }
      else
        frames = (Frame **) tmp;
      frames[count-1] = frame;
    }
  pclose (stream);

  *frame_count = count;
  return frames;
}

int
main (int argc, char **argv)
{
  int x, y, count, i;
  Frame **frames;
  if (get_pointer_coords (&x, &y))
    printf ("Mouse coords: %d, %d\n", x, y);

  if ((frames = get_frames (&count)) != NULL)
    for (i = 0; i < count; i++)
      printf ("x: %d, y: %d, width: %d, height: %d\n",
              frames[i]->x, frames[i]->y,
              frames[i]->width, frames[i]->height);
  return 0;
}
