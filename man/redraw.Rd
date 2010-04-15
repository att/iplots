\name{redraw}
\alias{redraw}
\title{
Redraw an object
}
\description{
\code{redraw} method redraws an object to make sure its visual
representation reflects its internal state.
}
\usage{
redraw(x, ...)
}
\arguments{
  \item{x}{object to redraw}
  \item{\dots}{optional arguments}
}
\details{
  Most object perform redraws implicitly, but sometimes is it more
  efficient to add or modify multiple objects without redrawing and
  issue a redraw at the end of such an operation.
}
\value{
  \code{x} possibly (but not commonly) modified to reflect its current
  state.
}
%\seealso{
%% ~~objects to See Also as \code{\link{help}}, ~~~
%}
\examples{
c = icontainer(frame=c(0, 0, 400, 300))
p = iplot(rnorm(100), rnorm(100), "x", "y", window=FALSE)

# add the plot to the container and redraw the container
redraw(c + p)
}
\keyword{iplot}
