\name{primitives}
\alias{primitives}
\title{
Obtain interactive primitives from a plot.
}
\description{
\code{primitives} function retrieves the objects representing plot
primitives in a plot.
}
\usage{
primitives(plot)
}
\arguments{
  \item{plot}{plot to query}
}
\details{
  Primitives are graphics objects added to an interactive plot
  explicitly using commands such as \code{\link{ilines}} or implicitly
  - e.g. by adding models to a plot. Primitives can be modified and
  interacted with.
}
\value{
  list of primitives
}
%\seealso{
%}
%\examples{
%}
\keyword{iplot}
