\name{xmlaClose}
\alias{xmlaClose}

\title{Closing an XMLA Session}
\description{
	Ending opened XMLA sessions.
}
\usage{
	xmlaClose(handle)
}
\arguments{
	\item{handle}{Connection object of class "XMLA" returned by \code{xmlaConnect}.}
}
\details{
	Closes an XMLA session (if supported by service) and frees up resources.
}
\value{
	\item{TRUE}{if session successfully closed.}
	\item{FALSE}{if error occurred while trying to close session.}
}
\seealso{
	\code{\link{xmlaConnect}}, \code{\link{xmlaExecute}}, \code{\link{xmlaDiscover}}
}
\keyword{IO}
\keyword{database}
\keyword{ReX}