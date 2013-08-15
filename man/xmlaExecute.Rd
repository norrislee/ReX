\name{xmlaExecute}
\alias{xmlaExecute}

\title{XMLA Execute Method}
\description{
    Sends XMLA commands to an XMLA service involving retrieving and updating data on the server.
}
\usage{
    xmlaExecute(handle, query, propertyList="")
}
\arguments{
    \item{handle}{Connection object of class "XMLA" returned by \code{xmlaConnect}.}
    \item{query}{A valid MDX query to be executed on the XMLA server.}
    \item{propertyList}{Semicolon separated key=value pairs of properties. See example.}
}
\details{
    Calls the XMLA Execute method to execute the MDX query against a certain XMLA server specified by the 
	handle. Properties may be defined in the \code{propertyList}, otherwise defaults to no properties.
}
\value{
    A data frame if data is returned from Execute method. \code{TRUE} if no data query is to update data 
	on server (ie. no data returned). \code{FALSE} if an error occurred.
}
\note{
    If returning an MDDataSet, first column of returned data frame contains the row names. If there are
	multiple dimensions on an axis, they will become a single comma-separated string per column/row.\cr
	MDDataSets currently only accept doubles (numerics).
}
\seealso{
    \code{\link{xmlaConnect}}, \code{\link{xmlaClose}}, \code{\link{xmlaDiscover}}
}
\examples{\dontrun{
    myData <- xmlaExecute(handle, "myQuery", propertyList="Catalog=Adventure Works DW;Format=Multidimensional")
}}
\keyword{IO}
\keyword{database}