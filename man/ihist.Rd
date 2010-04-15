\name{ihist}
\alias{ihist}
\alias{ihist.default}
\title{
Create an interactive histogram or spinogram
}
\description{
\code{ihist} creates an interactive histogram or spinogram
}
\usage{
ihist(x, ...)
ihist.default(x, xname = deparse(substitute(x)), ..., window, frame, flags)
}
\arguments{
\item{x}{a vector to create a histogram from}
\item{xname}{name of the x variable}
\item{\dots}{further optional arguments}
\item{window}{window in which the plot will be placed or \code{FALSE}
  for off-screen plots. If not specified the plot will create a new
  window and fill it with itself.}
\item{frame}{optional numeric vector with entries c(x, y, width, height)}
\item{flags}{optional flags specifying the behavior when the
  enclosing window is resized - see \code{\link{iconatiner}}}
}
%\details{
%}
\value{
Object of the class \code{iHistogram}. Virtual attributes common to all
plots are supported as well as additional ones mentioned below.
}
\section{Virtual Attributes}{
  In addition ot the virutal attributes common to all plots histogram
  also support the following:
  \describe{
    \item{bin.width}{bin width (scalar numeric}
    \item{bins}{number of bins}
    \item{anchor}{archor value fo the first bin}
    \item{spines}{\code{TRUE} for spinogram, \code{FALSE} for histogram
      (c.f. \code{\link{ibar}})}
  }
}
\seealso{
\code{\link{iplot}}, \code{\link{ibar}}, \code{\link{icontainer}}
}
\examples{
h = ihist(rnorm(1e5))
h$bin.width
h$bins
h$bin.width = 0.2
}
\keyword{hplot}
