\name{ipcp}
\alias{ipcp}
\alias{ipcp.default}
\title{
Create an interactive parallel coordinates plot
}
\description{
\code{ipcp} creates an interactive parallel coordinates plot
}
\usage{
ipcp(x, ...)
\method{ipcp}{default}(x, ..., window, frame, flags) 
}
\arguments{
  \item{x}{data frame, list of variables or the first column to plot}
  \item{\dots}{additional parameters or coordinates}
  \item{window}{window in which the plot will be placed or \code{FALSE}
    for off-screen plots. If not specified the plot will create a new
    window and fill it with itself.}
  \item{frame}{optional numeric vector with entries c(x, y, width,
    height)}
  \item{flags}{optional flags specifying the behavior when the
    enclosing window is resized - see \code{\link{icontainer}}}
}
%\details{
%}
\value{
Object of the class \code{iPCP} (subclass of \code{iPlot}).
}
\section{Interaction}{
  Left and right \code{Arrow} keys (aka cursor keys) can be used to
  control the opacity of the lines.
}
\seealso{
\code{\link{iplot}}, \code{\link{ihist}}, \code{\link{ibar}}
}
\examples{
data(mtcars)
ipcp(mtcars)
}
\keyword{hplot}
