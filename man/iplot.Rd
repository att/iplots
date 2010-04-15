\name{iplot}
\alias{iplot}
\alias{iplot.default}
\title{
Creates an interactive plot
}
\description{
\code{iplot} method is called for its side-effect of creating an interactive
plot. It is the interactive version of the well-known \code{\link{plot}}
method.
}
\usage{
iplot(x, ...)
iplot.default(x, y, xname = deparse(substitute(x)), yname = deparse(substitute(y)),
    ..., window, frame, flags)
}
\arguments{
  \item{x}{object to plot (in the generic) or the x coordinates for the
    default xy plot}
  \item{y}{y coordinates of the point to plot}
  \item{xname}{name of the x variable}
  \item{yname}{name of the y variable}
  \item{\dots}{further optional arguments}
  \item{window}{window in which the plot will be placed. If not
    spcieified the plot will create a new window and fill it with
    itself.}
  \item{frame}{optional numeric vector with entries c(x, y, width, height)}
  \item{flags}{optional flags specifying the behavior when the enclosing
    window is resized}
}
\details{
  \code{iplot} is a generic method which can be implemented for
  numberous objects.

  \code{iplot.default} is the default implementation creating a 2d
  scatterplot.

  All plots of the class \code{iPlot} support virtual attributes.
}
\value{
Returns an object that is a subclass of \code{iPlot} (the actual class
will depend on the type of plot created). This object it can be used to
modify the parameters of the plot, to iteract with the plot and to add
other components to the plot.
}
%\references{
%}
%\author{
%}
\seealso{
\code{\link{ihist}}, \code{\link{ibar}}, \code{\link{ipcp}}
}
\examples{
x = rnorm(1000)
y = rnorm(1000)/4 + x
p = iplot(x,y)
p$xlim
p$xlim=c(-3.5,3.5)
m = lm(y ~ x)
l = iabline(m)
l$color = 2
}
\keyword{hplot}
