#include "ReX.h"
#include <algorithm>
#include <string>

typedef struct ConnectionHandle {
	const char *connectionString;
	const char *userName;
	const char *password;
	std::string sessionID;

	ConnectionHandle(const char *endpoint, const char *uid, const char *pw, std::string session)
		:connectionString(endpoint), userName(uid), password(pw), sessionID(session) {}
} XMLAHandle;

int numChannels = 0;

RcppExport SEXP RXMLAConnect(SEXP connection, SEXP uid, SEXP pw)
{
	XmlaWebServiceSoapProxy service = XmlaWebServiceSoapProxy(SOAP_XML_DEFAULTNS, SOAP_XML_DEFAULTNS);

	const char *connectionString = CHAR(STRING_ELT(connection,0));

	ns1__BeginSession beginSession;
	service.soap_header(&beginSession, NULL, NULL, NULL);

	_ns1__Execute execute;
	ns1__CommandStatement command;
	ns1__Properties properties;
	ns1__PropertyList propertyList;
	_ns1__ExecuteResponse response;
	std::string statement;

	execute.Command = &command;
	command.Statement = &statement;
	execute.Properties = &properties;
	properties.PropertyList = &propertyList;
	service.userid = CHAR(STRING_ELT(uid,0));
	service.passwd = CHAR(STRING_ELT(pw,0));

	if (service.Execute(connectionString, NULL, &execute, &response) == SOAP_OK)
	{
		XMLAHandle *handle;
		handle = new ConnectionHandle(connectionString, CHAR(STRING_ELT(uid,0)), CHAR(STRING_ELT(pw,0)), *service.header->ns1__Session_->SessionId);
		Rcpp::XPtr<XMLAHandle> ptr(handle, true);
		Rcpp::IntegerVector ret(1);
		ret[0] = ++numChannels;
		ret.attr("Endpoint") = connectionString;
		ret.attr("Username") = CHAR(STRING_ELT(uid,0));
		ret.attr("Password") = "********";
		ret.attr("Session") = *service.header->ns1__Session_->SessionId;
		ret.attr("Pointer") = ptr;
		std::cout << "Connection successful" << std::endl;
		return ret;
	}
	char * errorMessage = service.fault->faultstring;
	std::cerr << errorMessage << std::endl;
	return R_NilValue;
}

RcppExport SEXP RXMLAClose(SEXP handle)
{
	XmlaWebServiceSoapProxy service = XmlaWebServiceSoapProxy(SOAP_XML_DEFAULTNS, SOAP_XML_DEFAULTNS);

	Rcpp::XPtr<XMLAHandle> ptr(handle);
	const char *connectionString = ptr->connectionString;

	ns1__EndSession endSession;
	std::string sessionId = ptr->sessionID;
	endSession.SessionId = &sessionId;
	service.soap_header(NULL, &endSession, NULL, NULL);

	_ns1__Execute execute;
	ns1__CommandStatement command;
	ns1__Properties properties;
	ns1__PropertyList propertyList;
	_ns1__ExecuteResponse response;
	std::string statement;

	execute.Command = &command;
	command.Statement = &statement;
	execute.Properties = &properties;
	properties.PropertyList = &propertyList;
	service.userid = ptr->userName;
	service.passwd = ptr->password;

	if (service.Execute(connectionString, NULL, &execute, &response) == SOAP_OK)
	{
		std::cout << "Session ended" << std::endl;
		return Rcpp::wrap(true);
	}
	char * errorMessage = service.fault->faultstring;
	std::cerr << errorMessage << std::endl;
	return Rcpp::wrap(false);
}

void parseKeyValuePairs(std::string *kvString, std::vector<char *>& vector)
{
	std::size_t frontIndex1 = 0;
	std::size_t frontIndex2 = 0;
	std::size_t backIndex = 0;
	int counter = 0;
	do
	{
		if (counter > 50)
			break;
		frontIndex1 = kvString->find("=", backIndex);
		std::cout << frontIndex1 << std::endl;
		std::string key = kvString->substr(backIndex, frontIndex1 - backIndex);
		backIndex = frontIndex1 + 1;
		frontIndex2 = kvString->find(";", backIndex);
		std::cout << frontIndex2 << std::endl;
		std::string value = kvString->substr(backIndex, frontIndex2 - backIndex);
		backIndex = frontIndex2 + 1;
		std::string *xmlProperty = new std::string;
		*xmlProperty = "<" + key + ">" + value + "</" + key + ">";
		std::cout << *xmlProperty << std::endl;
		vector.push_back(strdup(xmlProperty->c_str()));
		counter++;
	}
	while (frontIndex1 <= (kvString->length() - 2) && frontIndex2 <= (kvString->length() - 2));
}

void mdDataSetGetNames(Rcpp::CharacterVector &names, ns4__Axes *axes, int i, bool isRow)
{
	int axis;
	if (isRow)
		axis = 1;
	else 
		axis = 0;
	std::string name = "";
	std::vector<ns4__Member *> memberList = axes->Axis[axis]->__union_Axis->Tuples->Tuple[i]->Member;
	for (int j = 0; j < memberList.size(); j++)
	{
		name = name + *memberList[j]->Caption + ", ";
	}
	names.push_back(name.substr(0, name.size() - 2));
}

void rowSetParseData(std::vector<ns2__Row *> rows, Rcpp::DataFrame *resultDataFrame, char *colName, bool isChar)
{
	rapidxml::xml_document<> data;
	Rcpp::CharacterVector dimension;
	Rcpp::NumericVector dataColumn;
	for (int row = 0; row < rows.size(); row++)
	{
		bool found = false;
		for (int rowData = 0; rowData < rows[row]->__any.size(); rowData++)
		{
			char *parseText;
			int textLength = strlen(rows[row]->__any[rowData]);
			parseText = new char[textLength+1];
			parseText = strcpy(parseText, rows[row]->__any[rowData]);
			data.parse<0>(parseText);
			if (strcmp(data.first_node()->name(),colName) == 0)
			{
				if (isChar)
					dimension.push_back(data.first_node()->value());
				else
					dataColumn.push_back(atof(data.first_node()->value()));
				found = true;
				break;
			}
		}
		if (!found && isChar)
			dimension.push_back(NA_STRING);
		else if (!found && !isChar)
			dataColumn.push_back(NA_REAL);
	}
	if (isChar)
		resultDataFrame->push_back(dimension);
	else
		resultDataFrame->push_back(dataColumn);
}

RcppExport SEXP RXMLAExecute(SEXP handle, SEXP query, SEXP rPropertiesString)
{
	XmlaWebServiceSoapProxy service = XmlaWebServiceSoapProxy(SOAP_XML_DEFAULTNS, SOAP_XML_DEFAULTNS);

	Rcpp::XPtr<XMLAHandle> ptr(handle);
	const char *connectionString = ptr->connectionString;
	std::string propertiesString = CHAR(STRING_ELT(rPropertiesString,0));

	ns1__Session session;
	std::string sessionId = ptr->sessionID;
	session.SessionId = &sessionId;
	service.soap_header(NULL, NULL, &session, NULL);

	_ns1__Execute execute;
	ns1__CommandStatement command;
	ns1__Properties properties;
	ns1__PropertyList propertyList;
	_ns1__ExecuteResponse response;

	std::string statement = CHAR(STRING_ELT(query,0));
	command.Statement = &statement;
	execute.Command = &command;
	execute.Properties = &properties;
	properties.PropertyList = &propertyList;
	if (!propertiesString.empty())
	{
		parseKeyValuePairs(&propertiesString, propertyList.__any);
	}
	service.userid = ptr->userName;
	service.passwd = ptr->password;

	if (service.Execute(connectionString, NULL, &execute, &response) == SOAP_OK)
	{
		// Parse MDDataSet
		if (response.return_->ns4__root != NULL && response.return_->ns4__root->__union_ResultXmlRoot != NULL && response.return_->ns4__root->__union_ResultXmlRoot->Axes != NULL) 
		{
			if (response.return_->ns4__root->__union_ResultXmlRoot->Axes->Axis.size() < 3)
			{
				std::cerr << "Error: No data on Axis1" << std::endl;
				return R_NilValue;
			}
			if (response.return_->ns4__root->__union_ResultXmlRoot->Axes->Axis.size() > 3)
			{
				std::cerr << "Error: More than 2 axes not supported" << std::endl;
				return R_NilValue;
			}

			ns4__Axes *axes = response.return_->ns4__root->__union_ResultXmlRoot->Axes;
			std::vector<ns4__Cell *> cellDataVector = response.return_->ns4__root->__union_ResultXmlRoot->CellData->Cell;
			int numCols = response.return_->ns4__root->__union_ResultXmlRoot->Axes->Axis[0]->__union_Axis->Tuples->Tuple.size();
			int numRows = response.return_->ns4__root->__union_ResultXmlRoot->Axes->Axis[1]->__union_Axis->Tuples->Tuple.size();
			int cellDataVectorMember = 0;

			Rcpp::CharacterVector colNames;
			Rcpp::CharacterVector rowNames;
			Rcpp::NumericMatrix resultMatrix(numRows, numCols);

			for (int row = 0; row < numRows; row++)
			{
				for (int col = 0; col < numCols; col++)
				{
					if (cellDataVector[cellDataVectorMember]->CellOrdinal == ((row * numCols) + col))
					{
						resultMatrix(row, col) = *cellDataVector[cellDataVectorMember]->Value;
						if (cellDataVectorMember < cellDataVector.size() - 1)
						{
							cellDataVectorMember += 1;
						}
					}
					else
					{
						resultMatrix(row, col) = NA_REAL;
					}
				}
				mdDataSetGetNames(rowNames, axes, row, true);
			}

			for (int col = 0; col < numCols; col++)
			{
				mdDataSetGetNames(colNames, axes, col, false);
			}

			colNames.push_front("Row Names");
			Rcpp::DataFrame resultDataFrame(resultMatrix);
			resultDataFrame.push_front(rowNames);
			resultDataFrame.attr("names") = colNames;
			service.destroy();
			return resultDataFrame;
		}
		// Parse RowSet
		else if (response.return_->ns2__root != NULL && response.return_->ns2__root->xsd__schema != NULL 
			&& response.return_->ns2__root->__union_ResultXmlRoot != NULL && !response.return_->ns2__root->__union_ResultXmlRoot->row.empty())
		{
			std::string rawXML = "<root xmlns=\"urn:schemas-microsoft-com:xml-analysis:rowset\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"><xsd:schema targetNamespace=\"urn:schemas-microsoft-com:xml-analysis:rowset\" xmlns:sql=\"urn:schemas-microsoft-com:xml-sql\" elementFormDefault=\"qualified\">";
			rawXML = rawXML + response.return_->ns2__root->xsd__schema + "</xsd:schema></root>";
			char *schema = strdup(rawXML.c_str());
			rapidxml::xml_document<> doc;
			doc.parse<0>(schema);

			// Find XML section containing column names
			rapidxml::xml_node<char> *rowNode = doc.first_node()->first_node()->first_node("xsd:complexType");
			while(rowNode != NULL && strcmp(rowNode->first_attribute("name")->value(), "row") != 0)
			{
				rowNode = rowNode->next_sibling("xsd:complexType");
			}

			rapidxml::xml_node<char> *schemaElementNode = rowNode->first_node()->first_node();
			std::vector<ns2__Row *> rows = response.return_->ns2__root->__union_ResultXmlRoot->row;
			Rcpp::DataFrame resultDataFrame;
			rapidxml::xml_document<> data;
			Rcpp::CharacterVector colNames;
			char *colName;

			while(schemaElementNode != NULL)
			{
				colName = schemaElementNode->first_attribute("name")->value();
				colNames.push_back(colName);
				if (schemaElementNode->first_attribute("type") != 0)
				{
					rowSetParseData(rows, &resultDataFrame, colName, true);
				}
				else
				{
					rowSetParseData(rows, &resultDataFrame, colName, false);
				}
				schemaElementNode = schemaElementNode->next_sibling();
			}
			resultDataFrame.attr("names") = colNames;

			service.destroy();
			return resultDataFrame;
		}
		service.destroy();
		return Rcpp::wrap(true);
	}
	else 
	{
		char * errorMessage = service.fault->detail->__any;
		std::cerr << errorMessage << std::endl;
	}
	service.destroy();
	return R_NilValue;
}

RcppExport SEXP RXMLADiscover(SEXP handle, SEXP request, SEXP rRestrictionsString, SEXP rPropertiesString)
{
	XmlaWebServiceSoapProxy service = XmlaWebServiceSoapProxy(SOAP_XML_DEFAULTNS, SOAP_XML_DEFAULTNS);

	Rcpp::XPtr<XMLAHandle> ptr(handle);
	const char *connectionString = ptr->connectionString;
	std::string propertiesString = CHAR(STRING_ELT(rPropertiesString,0));
	std::string restrictionsString = CHAR(STRING_ELT(rRestrictionsString, 0));

	ns1__Session session;
	std::string sessionId = ptr->sessionID;
	session.SessionId = &sessionId;
	service.soap_header(NULL, NULL, &session, NULL);

	_ns1__Discover discover;
	ns1__Restrictions restrictions;
	ns1__RestrictionList restrictionList;
	ns1__Properties properties;
	ns1__PropertyList propertyList;
	_ns1__DiscoverResponse discoverResponse;

	std::string requestType = CHAR(STRING_ELT(request,0));
	std::transform(requestType.begin(), requestType.end(), requestType.begin(), ::toupper);
	discover.RequestType = &requestType;
	discover.Restrictions = &restrictions;
	restrictions.RestrictionList = &restrictionList;
	discover.Properties = &properties;
	properties.PropertyList = &propertyList;
	if (!propertiesString.empty())
	{
		parseKeyValuePairs(&propertiesString, propertyList.__any);
	}
	if (!restrictionsString.empty())
	{
		parseKeyValuePairs(&restrictionsString, restrictionList.__any);
	}
	service.userid = ptr->userName;
	service.passwd = ptr->password;

	if (service.Discover(connectionString, NULL, &discover, &discoverResponse) == SOAP_OK)
	{
		if (discoverResponse.return_->ns2__root != NULL && discoverResponse.return_->ns2__root->__union_ResultXmlRoot != NULL && !discoverResponse.return_->ns2__root->__union_ResultXmlRoot->row.empty())
		{
			std::vector<ns2__Row *> rows = discoverResponse.return_->ns2__root->__union_ResultXmlRoot->row;
			Rcpp::List resultList;
			for (int i = 0; i < rows.size(); i++)
			{
				std::vector<char *> row = rows[i]->__any;
				Rcpp::CharacterVector rowData;
				for (std::vector<char *>::iterator iter = row.begin(); iter != row.end(); iter++)
				{
					rowData.push_back(*iter);
				}
				resultList.push_back(rowData);
			}
			service.destroy();
			return resultList;
		}
	}
	else
	{
		std::cerr << service.fault->faultstring << std::endl;
	}
	service.destroy();
	return R_NilValue;
}

RcppExport SEXP RXMLAValidHandle(SEXP handle)
{
	Rcpp::XPtr<XMLAHandle> ptr(handle);

	return Rcpp::wrap(ptr && TYPEOF(ptr) == EXTPTRSXP);
}