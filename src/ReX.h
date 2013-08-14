#ifndef _REX_REX_H
#define _REX_REX_H

#include <Rcpp.h>
#include "soapXmlaWebServiceSoapProxy.h"
#include "XmlaWebServiceSoap.nsmap"
#include "rapidxml.hpp"

RcppExport SEXP RXMLAConnect(SEXP connection, SEXP uid, SEXP pw);

RcppExport SEXP RXMLAClose(SEXP handle);

RcppExport SEXP RXMLAExecute(SEXP handle, SEXP query, SEXP rPropertiesString);

RcppExport SEXP RXMLADiscover(SEXP handle, SEXP request, SEXP rRestrictionsString, SEXP rPropertiesString);

#endif