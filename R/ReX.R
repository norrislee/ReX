xmlaConnect <- function(url, uid="", pwd="")
{
	connection <- .Call("RXMLAConnect", as.character(url), as.character(uid), as.character(pwd))
	if (!is.null(connection))
		class(connection) <- "XMLA"
	connection
}

xmlaClose <- function(sessionID)
{
	.Call("RXMLAClose", as.character(sessionID))
}

xmlaExecute <- function(url, sessionId, uid, pwd, query, ...)
{
	.Call("RXMLAExecute", as.character(url), as.character(sessionId), as.character(uid), as.character(pwd), as.character(query))
}

xmlaDiscover <- function(url, uid="", pwd="", requestType, ...)
{
	.Call("RXMLADiscover", as.character(url), as.character(uid), as.character(pwd), as.character(requestType))
}

print.XMLA <- function(x, ...)
{
    cat("Connection \nEndpoint: ", attr(x, "Endpoint"), "\nSessionID: ", attr(x, "Session"), "\nUsername: ", attr(x, "Username"), "\nPassword: ", attr(x, "Password"), "\n")
    invisible(x)
}