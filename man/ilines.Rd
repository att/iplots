\name{ilines}
\alias{ilines}
\alias{ilines.default}
\title{
Add lines to an interactive plot
}
\description{
\code{ilines} method adds lines objects to an interactive plot
}
\usage{
ilines(x, ...)
ilines.default(x, y, col, ..., plot = .Last.plot)
}
\arguments{
  \item{x}{specification of the lines (generic) or numeric vector of the
    x coordinates (default method)}
  \item{y}{numeric vector of the y coordinates}
  \item{col}{color of the line}
  \item{\dots}{further optional arguments}
  \item{plot}{plot to add the lines to}
}
\details{
  \code{ilines} is a shorthand for creating \code{iPolygon} object a and
  adding it to the plot.
}
\value{
  object of the class \code{iPolygon} which has been added to the
  plot. The object can be used to subsequently modify the lines and
  their attributes.
}  
%\seealso{
%% ~~objects to See Also as \code{\link{help}}, ~~~
%}
%\examples{
%}
\keyword{iplot}
