\name{ibar}
\alias{ibar}
\alias{ibar.factor}
\title{
Create an interactive bar chart or spine plot
}
\description{
\code{ibar} method creates an interactive bar chart
}
\usage{
ibar(x, ...)
\method{ibar}{factor}(x, xname = deparse(substitute(x)), ..., window, frame, flags)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{x}{factor to plot}
  \item{xname}{name of the x variable}
  \item{\dots}{further optional arguments}
  \item{window}{window in which the plot will be placed. If not
    specified the plot will create a new window and fill it with
    itself.}
  \item{frame}{optional numeric vector with entries c(x, y, width, height)}
  \item{flags}{optional flags specifying the behavior when the
    enclosing window is resized}
}
%\details{
%}
\value{
  Returns an object of class \code{iBarchart} (a subclass of
  \code{iPlot}). This object it can be used to modify the parameters of
  the plot, to iteract with the plot and to add other components to the
  plot.
}
\note{It is possible to switch between barchart and spineplot using the
  \code{S} keyboard key or the \code{spines} virtual attribute
  (\code{TRUE} for spineplot, \code{FALSE} for barchart).
}
\seealso{
  \code{\link{iplot}}, \code{\link{ihist}}, \code{\link{ipcp}}
}
\examples{
data(mtcars)
attach(mtcars)
iplot(hp, mpg)
b = ibar(factor(cyl))
select(b, carb > 2)
b$spines = TRUE
}
\keyword{hplot}
