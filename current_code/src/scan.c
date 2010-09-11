/*
 * scan.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * scan.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * scan.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <FreeImage.h>

#include "config.h"
#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#ifdef CAN_SCAN
#include <sane/sane.h>
#endif // CAN_SCAN //
#include <stdio.h>
#include <memory.h>
#include <string.h>
#ifdef CAN_OCR
#include "ocr_plug.h"
#include <pthread.h>
#endif // CAN_OCR //
#include "main.h"
#include "scan.h"
#include "db.h"
#include "doc_editor.h"
#include "utils.h"
#ifdef CAN_READODF
#include "read_odf.h"
#endif // CAN_READODF //
#include "debug.h"
#include "config.h"

#ifdef CAN_SCAN
const SANE_Device **device_list;
#endif // CAN_SCAN //
GHashTable *SCANWIDGETS;


#ifdef CAN_READODF
void importFile(GtkWidget *noUsed, GtkWidget *fileChooser) {

  char *fileName, *sql, *tmp, *dateStr;
  int lastInserted;
  GtkWidget *widget;
  GTimeVal todaysDate;

  fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileChooser));

  // Save Record
  debug_message("Saving record", DEBUGM);
  g_get_current_time (&todaysDate);
  dateStr = g_time_val_to_iso8601 (&todaysDate);

  sql = g_strdup("INSERT INTO docs \
    (doneocr, ocrtext, entrydate, filetype) \
    VALUES (0, '--fromDoc--', '");
  tmp = g_strconcat(dateStr, "', ", NULL);
  conCat(&sql, tmp);
  free(tmp);
  tmp = itoa(DOC_FILETYPE, 10);
  conCat(&sql, tmp);
  free(tmp);
  conCat(&sql, ") ");
  debug_message(sql, DEBUGM);
  runquery_db("1", sql);
  lastInserted = last_insert();
  free(sql);
  sql = itoa(lastInserted, 10);

  tmp = g_strdup(BASE_DIR);
  conCat(&tmp, "scans/");
  conCat(&tmp, sql);
  conCat(&tmp, ".odt");
  debug_message(tmp, DEBUGM);
  fcopy(fileName, tmp);
  free(tmp);

  // Open the document for editing.

}
#endif // CAN_READODF //

#ifdef CAN_SCAN
extern void doScanningOperation(struct scanParams *scanParam) {

  debug_message("made it to the Scanning Code", DEBUGM);

  SANE_Status status;
  SANE_Handle *openDeviceHandle;
  SANE_Byte *buffer;
  SANE_Int buff_len;
  SANE_Parameters pars;
  FILE *file;
  double c=0, totbytes=0, progress_d=0; 
  int q=0, hlp=0, resolution=0, paramSetRet=0, pages=0, pageCount=0,
  scan_bpl=0L, scan_ppl=0L, scan_lines=0, lastInserted=0, shoulddoocr=0, progress=0;
#ifdef CAN_OCR
  unsigned char *pic=NULL;
  int i=0;
  struct scanCallInfo infoData;
#endif // CAN_OCR //
  char *ocrText, *sql, *progressUpdate, *dateStr, *tmp, *tmp2, *id;
  GTimeVal todaysDate;
  GList *vars = NULL;
  char *devName, *uuid;

  devName = scanParam->devName;
  uuid = scanParam->uuid;
  resolution = scanParam->resolution;
  hlp = scanParam->hlp;
  q = scanParam->q;
  pageCount = scanParam->pages;

  // Open the device
  debug_message("sane_open", DEBUGM);
  status = sane_open ((SANE_String_Const) devName, (SANE_Handle)&openDeviceHandle);
  if(status != SANE_STATUS_GOOD) {
    debug_message("Cannot open device", ERROR);
    debug_message(devName, ERROR);
    return;
  }

  // Set Resolution 
  debug_message("Set resolution", DEBUGM);
  resolution = resolution*q;
  status = sane_control_option (openDeviceHandle, hlp, SANE_ACTION_SET_VALUE, &resolution, &paramSetRet);
  if(status != SANE_STATUS_GOOD) {
    debug_message("Cannot set resolution", ERROR);
    printf("sane error = %d (%s), return code = %d\n", status, sane_strstatus(status), paramSetRet);
    return;
  }


  // Get scanning params (from the scanner)
  debug_message("Get scanning params", DEBUGM);
  status = sane_get_parameters (openDeviceHandle, &pars);
  fprintf (stderr,
    "Parm : stat=%s form=%d,lf=%d,bpl=%d,pixpl=%d,lin=%d,dep=%d\n",
    sane_strstatus (status),
    pars.format, pars.last_frame,
    pars.bytes_per_line, pars.pixels_per_line,
    pars.lines, pars.depth);


  debug_message("sane_start", DEBUGM);
  status = sane_start (openDeviceHandle);
  if(status == SANE_STATUS_GOOD) {
    // Acquire Image & Save Document
    if ((file = fopen("/tmp/tmp.pnm", "w")) == NULL)
      debug_message("could not open file for output", ERROR);
    fprintf (file, "P5\n# SANE data follows\n%d %d\n%d\n", 
      pars.pixels_per_line, pars.lines*pageCount,
      (pars.depth <= 8) ? 255 : 65535);

    totbytes = pars.bytes_per_line * pars.lines * pageCount;
    pages = 1;
    c = 0;
    debug_message("scan_read - start", DEBUGM);

    progressUpdate = g_strdup("UPDATE scanprogress SET progress = ? WHERE client_id = ? ");
    int buffer_s = q;
    buffer = malloc(buffer_s);

    do {
      status = sane_read (openDeviceHandle, buffer, buffer_s, &buff_len);
      if (status != SANE_STATUS_GOOD) {
        if (status == SANE_STATUS_EOF) {
          pages++;
          if(pages > pageCount) 
            break;
          else {
            // Reset the scanner, ask for user confirmation of readyness.
            debug_message("sane_cancel", DEBUGM);
            sane_cancel (openDeviceHandle);
            tmp = g_strdup("Insert page ");
            tmp2 = itoa(pages, 10);
            conCat(&tmp, tmp2);
            debug_message(tmp, ERROR);
            free(tmp);
            debug_message("sane_start", DEBUGM);
            sane_start (openDeviceHandle);
          }
        }
      }

      if(buff_len > 0) {
        c += buff_len;

        // Update the progress info
        progress_d = 100 * (c / totbytes);
        progress = progress_d;
        if(progress > 100)
          progress = 100;

        vars = NULL;
        vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
        vars = g_list_append(vars, GINT_TO_POINTER(progress));
        vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
        vars = g_list_append(vars, g_strdup(uuid));
        runUpdate_db(progressUpdate, vars);

        fwrite (buffer, 1, buff_len, file);
      }
    } while (1);
    debug_message("scan_read - end", DEBUGM);
    fclose(file);
    free(progressUpdate);
    free(buffer);

    debug_message("sane_cancel", DEBUGM);
    sane_cancel(openDeviceHandle);
    scan_bpl = pars.bytes_per_line;
    scan_ppl = pars.pixels_per_line;
    scan_lines = pars.lines;
  }
  debug_message("sane_close", DEBUGM);
  sane_close(openDeviceHandle);
  debug_message("sane_exit", DEBUGM);
  sane_exit();


  // Save Record
  //
  debug_message("Saving record", DEBUGM);
  g_get_current_time (&todaysDate);
  dateStr = g_time_val_to_iso8601 (&todaysDate);

  ocrText = g_strdup("");

  sql = g_strdup("INSERT INTO docs \
    (doneocr, ocrtext, depth, lines, ppl, resolution, pages, entrydate, filetype) \
    VALUES (?, ?, 8, ?, ?, ?, ?, ?, ?)");

  vars = NULL;
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(shoulddoocr?1:0));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, ocrText);
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(scan_lines));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(scan_ppl));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(resolution/q));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(pageCount));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, dateStr);
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(SCAN_FILETYPE));

  runUpdate_db(sql, vars);
  lastInserted = last_insert();
  id = itoa(lastInserted, 10);


  // Convert Raw into JPEG
  debug_message(g_strconcat(BASE_DIR,"scans/",id,".pnm", NULL), DEBUGM);

  FreeImage_Initialise(TRUE);

  char *outFilename = g_strconcat(BASE_DIR,"scans/",id,".jpg", NULL);
  FIBITMAP *bitmap = FreeImage_Load(FIF_PGM, "/tmp/tmp.pnm", 0);
  FreeImage_Save(FIF_JPEG, bitmap, outFilename, 90);

  FreeImage_DeInitialise();

  debug_message(outFilename, DEBUGM);
  free(outFilename);

  debug_message("Sanning All done.", DEBUGM);

}
#endif // CAN_SCAN //

