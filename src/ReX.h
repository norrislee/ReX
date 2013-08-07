#ifndef _REX_REX_H
#define _REX_REX_H

#include <Rcpp.h>
#include "soapXmlaWebServiceSoapProxy.h"
#include "XmlaWebServiceSoap.nsmap"

RcppExport SEXP RXMLAConnect(SEXP connection, SEXP uid, SEXP pw);

RcppExport void RXMLAClose(SEXP sessionId);

RcppExport SEXP RXMLAExecute(SEXP connection, SEXP sessionId, SEXP uid, SEXP pw, SEXP query);

RcppExport SEXP RXMLADiscover(SEXP connection, SEXP uid, SEXP pw, SEXP request);

#endif