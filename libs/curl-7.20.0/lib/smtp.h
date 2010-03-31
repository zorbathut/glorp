#ifndef __SMTP_H
#define __SMTP_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 2009, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: smtp.h,v 1.4 2009-12-30 22:50:42 bagder Exp $
 ***************************************************************************/

#include "pingpong.h"

/****************************************************************************
 * SMTP unique setup
 ***************************************************************************/
typedef enum {
  SMTP_STOP,        /* do nothing state, stops the state machine */
  SMTP_SERVERGREET, /* waiting for the initial greeting immediately after
                       a connect */
  SMTP_EHLO,
  SMTP_STARTTLS,
  SMTP_MAIL, /* MAIL FROM */
  SMTP_RCPT, /* RCPT TO */
  SMTP_DATA,
  SMTP_QUIT,
  SMTP_LAST  /* never used */
} smtpstate;

/* smtp_conn is used for struct connection-oriented data in the connectdata
   struct */
struct smtp_conn {
  struct pingpong pp;
  char *domain;    /* what to send in the EHLO */
  int eob;         /* number of bytes of the EOB (End Of Body) that has been
                      received thus far */
  smtpstate state; /* always use smtp.c:state() to change state! */
};

extern const struct Curl_handler Curl_handler_smtp;
extern const struct Curl_handler Curl_handler_smtps;

/* this is the 5-bytes End-Of-Body marker for SMTP */
#define SMTP_EOB "\x0d\x0a\x2e\x0d\x0a"
#define SMTP_EOB_LEN 5

/* if found in data, replace it with this string instead */
#define SMTP_EOB_REPL "\x0d\x0a\x2e\x2e"
#define SMTP_EOB_REPL_LEN 4

CURLcode Curl_smtp_escape_eob(struct connectdata *conn, int nread);

#endif /* __SMTP_H */
