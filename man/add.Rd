\name{add}
\alias{add}
\alias{add.default}
\alias{add.iContainer}
\alias{add.iPlot}
\alias{add.primitive}
\alias{delete}
\title{
  Add (or delete) objects into (or from) an enclosing object
}
\description{
  \code{add} method adds objects into an enclosing object, \code{delete}
  removes objects from its parents. The exact semantics varies depending
  on the objects involved.
}
\usage{
add(x, ...)
delete(x, ...)

\method{add}{iPlot}(x, obj, ...)
\method{add}{iContainer}(x, obj, ...)
\method{add}{primitive}(x, obj, ...)
\method{add}{default}(x, obj, ...)

}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{x}{target object}
  \item{obj}{object to be added into the target}
  \item{\dots}{further optional arguments}
}
\details{
The add method is defined in a very general way. The actual
implementatations usually forward the dispatch to the second argument
(at least the one listed here do) such that \code{add} methods can be
defined for any combination of classes for the \code{x} and \code{obj}
objects.

The use ranges from adding plots into containers, plot privitives to
plots or even models to plots. The goal of this method (and its inverse
\code{remove}) is to define the concept of adding or removing any kind
of object or property that makes sense.
}
\value{
The value is \code{x} modified to accomodate the addition of
\code{obj}. Depending on the semantics it may be (and often is) \code{x}
itself.
}
\note{
  The \code{+} and \code{-} operators are often dispatched to
  \code{add} and \code{delete} as a syntactic sugar for objects where
  such operation makes sense such that, for example, adding a model
  representation to a plot \code{p} can be done as \code{p + lm(y ~ x)}.

  (Also note that \code{remove} is a different non-generic function!)
}
%\seealso{
%}
%\examples{
%}
\keyword{iplot}
