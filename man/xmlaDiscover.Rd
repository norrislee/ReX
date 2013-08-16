\name{xmlaDiscover}
\alias{xmlaDiscover}

\title{XMLA Discover Method}
\description{
    Retrieves information from an XMLA server.
}
\usage{
    xmlaDiscover(handle, requestType, restrictionList="", propertyList="")
}
\arguments{
    \item{handle}{Connection object of class "XMLA" returned by \code{xmlaConnect}.}
	\item{requestType}{Character string that determines the type of metadata to be returned.}
	\item{restrictionList}{Semicolon separated key=value pairs of restrictions. See example.}
	\item{propertyList}{Semicolon separated key=value pairs of properties. See example.}
}
\details{
    Calls the XMLA Discover method to retrieve information or metadata from a certain XMLA server 
	specified by the handle. The data to be returned is determined by the requestType, filtered/modified 
	by restrictions and properties. Restrictions and properties may be defined in the 
	\code{restrictionList} and the \code{propertyList} respectively, otherwise defaults to none.
}
\value{
    A \code{"list"} object containing the data to be returned with each item in list corresponding to 
	a row. \code{FALSE} if an error occurred.
}
\seealso{
    \code{\link{xmlaConnect}}, \code{\link{xmlaClose}}, \code{\link{xmlaExecute}}
}
\examples{\dontrun{
    cubeList <- xmlaDiscover(handle, "MDSCHEMA_CUBES", 
	                restrictionList="CATALOG_NAME=Adventure Works DW",
		            propertyList="Catalog=Adventure Works DW;Format=Tabular")
}}
\keyword{IO}
\keyword{database}
\keyword{ReX}