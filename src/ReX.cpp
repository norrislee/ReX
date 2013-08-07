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
	const char *connectionString = CHAR(STRING_ELT(connection,0));
	XmlaWebServiceSoapProxy service = XmlaWebServiceSoapProxy(SOAP_XML_DEFAULTNS, SOAP_XML_DEFAULTNS);
	ns1__BeginSession beginSession;
	ns1__EndSession endSession;
	ns1__Session session;
	ns6__Security security;
	ns6__UsernameTokenElement usernameTokenElement;

	service.userid = CHAR(STRING_ELT(uid,0));
	service.passwd = CHAR(STRING_ELT(pw,0));

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

	if (service.Execute(connectionString, NULL, &execute, &response) == SOAP_OK)
	{
		XMLAHandle *handle;
		handle = new ConnectionHandle(connectionString, CHAR(STRING_ELT(uid,0)), CHAR(STRING_ELT(pw,0)), *service.header->ns1__Session_->SessionId);
		Rcpp::XPtr<XMLAHandle> ptr(handle, true);
		//Rcpp::CharacterVector connection;
		//connection.push_back(connectionString);
		//connection.push_back(CHAR(STRING_ELT(uid,0)));
		//connection.push_back(CHAR(STRING_ELT(pw,0)));
		//connection.push_back(*service.header->ns1__Session_->SessionId);
		Rcpp::IntegerVector ret(1);
		ret[0] = ++numChannels;
		ret.attr("Endpoint") = connectionString;
		ret.attr("Username") = CHAR(STRING_ELT(uid,0));
		ret.attr("Password") = "********";
		ret.attr("Session") = *service.header->ns1__Session_->SessionId;
		ret.attr("Pointer") = ptr;
		return ret;
	}
	char * errorMessage = service.fault->detail->__any;
	std::cerr << errorMessage << std::endl;
	return R_NilValue;
}

RcppExport void RXMLAClose(SEXP sessionId)
{

}

RcppExport SEXP RXMLAExecute(SEXP connection, SEXP sessionId, SEXP uid, SEXP pw, SEXP query)
{
	const char *connectionString = CHAR(STRING_ELT(connection,0));
	XmlaWebServiceSoapProxy service = XmlaWebServiceSoapProxy(SOAP_XML_DEFAULTNS, SOAP_XML_DEFAULTNS);
	
	ns1__Session session;
	std::string sessionString =CHAR(STRING_ELT(sessionId,0));
	session.SessionId = &sessionString;
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
	service.userid = CHAR(STRING_ELT(uid,0));
	service.passwd = CHAR(STRING_ELT(pw,0));

	if (service.Execute(connectionString, NULL, &execute, &response) == SOAP_OK)
	{
		if (response.return_->ns4__root != NULL && response.return_->ns4__root->__union_ResultXmlRoot != NULL && response.return_->ns4__root->__union_ResultXmlRoot->Axes != NULL) 
		{
			if (response.return_->ns4__root->__union_ResultXmlRoot->Axes->Axis[1]->name->compare("Axis1") != 0)
			{
				std::cerr << "Error: No data on Axis1" << std::endl;
				return R_NilValue;
			}
			ns4__Axes *axes = response.return_->ns4__root->__union_ResultXmlRoot->Axes;
			std::vector<ns4__Cell *> cellDataVector = response.return_->ns4__root->__union_ResultXmlRoot->CellData->Cell;
			int numCols = response.return_->ns4__root->__union_ResultXmlRoot->Axes->Axis[0]->__union_Axis->Tuples->Tuple.size();
			int numRows = response.return_->ns4__root->__union_ResultXmlRoot->Axes->Axis[1]->__union_Axis->Tuples->Tuple.size();
			int totalCellDataCount = response.return_->ns4__root->__union_ResultXmlRoot->CellData->Cell.size();
			int cellDataVectorMember = 0;

			Rcpp::CharacterVector colNames(numCols);
			Rcpp::CharacterVector rowNames(numRows);
			Rcpp::NumericMatrix resultMatrix(numRows, numCols);

			Rcpp::List dfList = Rcpp::List::create();
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

				std::string rowName = "";
				std::vector<ns4__Member *> memberList = axes->Axis[1]->__union_Axis->Tuples->Tuple[row]->Member;
				for (int i = 0; i < memberList.size(); i++)
				{
					rowName = rowName + *memberList[i]->Caption + ", ";
				}
				rowNames[row] = rowName.substr(0, rowName.size() - 2);
			}

			for (int col = 0; col < numCols; col++)
			{
				std::string colName = "";
				std::vector<ns4__Member *> memberList = axes->Axis[0]->__union_Axis->Tuples->Tuple[col]->Member;
				for (int i = 0; i < memberList.size(); i++)
				{
					colName = colName + *memberList[i]->Caption + ", ";
				}
				colNames[col] = colName.substr(0, colName.size() - 2);
			}

			Rcpp::DataFrame resultDataFrame(resultMatrix);
			resultDataFrame.attr("names") = colNames;
			resultDataFrame.attr("row.names") = rowNames;
			service.destroy();
			return resultDataFrame;
		}
	}
	else 
	{
		char * errorMessage = service.fault->detail->__any;
		std::cerr << errorMessage << std::endl;
	}
	service.destroy();
	return R_NilValue;
}

RcppExport SEXP RXMLADiscover(SEXP connection, SEXP uid, SEXP pw, SEXP request)
{
	const char *connectionString = CHAR(STRING_ELT(connection,0));

	XmlaWebServiceSoapProxy service = XmlaWebServiceSoapProxy(SOAP_XML_DEFAULTNS, SOAP_XML_DEFAULTNS);

	_ns1__Discover discover;
	std::string requestType = CHAR(STRING_ELT(request,0));
	std::transform(requestType.begin(), requestType.end(), requestType.begin(), ::toupper);
	ns1__Restrictions restrictions;
	ns1__RestrictionList restrictionList;
	ns1__Properties properties;
	ns1__PropertyList propertyList;

	_ns1__DiscoverResponse discoverResponse;

	discover.RequestType = &requestType;
	discover.Restrictions = &restrictions;
	restrictions.RestrictionList = &restrictionList;
	discover.Properties = &properties;
	properties.PropertyList = &propertyList;
	propertyList.__any.push_back("");

	service.userid = CHAR(STRING_ELT(uid,0));
	service.passwd = CHAR(STRING_ELT(pw,0));
	if (service.Discover(connectionString, NULL, &discover, &discoverResponse) == SOAP_OK)
	{
		std::cout << "<root>" << std::endl;
		std::vector<ns2__Row *> rows = discoverResponse.return_->ns2__root->__union_ResultXmlRoot->row;
		Rcpp::List resultList;
		for (int i = 0; i < rows.size(); i++)
		{
			std::cout << "\t<row>" << std::endl;
			std::vector<char *> row = rows[i]->__any;
			Rcpp::CharacterVector rowData;
			for (std::vector<char *>::iterator iter = row.begin(); iter != row.end(); iter++)
			{
				std::cout << "\t\t" << *iter << std::endl;
				rowData.push_back(*iter);
			}
			std::cout << "\t</row>" << std::endl;
			resultList.push_back(rowData);
		}
		std::cout << "</root>" << std::endl;
		service.destroy();
		return resultList;
	}
	else
	{
		char * errorMessage = service.fault->detail->__any;
		std::cerr << errorMessage << std::endl;
	}
	service.destroy();
	return R_NilValue;
}

