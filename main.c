#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RATPOISON "/usr/bin/ratpoison"

typedef struct {
  int x;
  int y;
  int width;
  int height;
} Frame;

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
  int i, count;
  Frame **frames;

  if ((frames = get_frames (&count)) != NULL)
    for (i = 0; i < count; i++)
      printf ("x: %d, y: %d, width: %d, height: %d\n",
              frames[i]->x, frames[i]->y,
              frames[i]->width, frames[i]->height);
  return 0;
}
