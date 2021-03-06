/*
 * ratcursor - a program to select a ratpoison frame with the rat
 *
 * Copyright (C) 2010  Lincoln de Sousa <lincoln@comum.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#define RATPOISON "/usr/bin/ratpoison"

typedef struct {
  int number;
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

  XCloseDisplay (display);
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
          if (strcmp (piece, ":number") == 0)
            frame->number = atoi (strtok_r (NULL, " ", &saveptr2));
          else if (strcmp (piece, ":x") == 0)
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

static int
select_frame (void)
{
  int x, y, count, i;
  Frame **frames;
  if (!get_pointer_coords (&x, &y))
    return 0;
  if ((frames = get_frames (&count)) == NULL)
    return 0;

  for (i = 0; i < count; i++)
    {
      if (x > frames[i]->x && x < frames[i]->x + frames[i]->width &&
          y > frames[i]->y && y < frames[i]->y + frames[i]->height)
        {
          char cmd[128];
          sprintf (cmd, RATPOISON " -c \"fselect %d\"", frames[i]->number);
          system (cmd);
          break;
        }
    }

  for (i = 0; i < count; i++)
    free (frames[i]);
  free (frames);
  return 1;
}

int
main (int argc, char **argv)
{
  select_frame ();
  return 0;
}
