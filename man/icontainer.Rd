\name{icontainer}
\alias{icontainer}
\title{
Creates a new interactive container
}
\description{
\code{icontainer} creates a new interactive container. Containers are
used to group interactive visual objects (such as plots) and can be
nested.
}
\usage{
icontainer(parent = NULL, window, frame, flags)
}
\arguments{
  \item{parent}{parent container to register with. If \code{NULL} this
    container is created on its own.}
  \item{window}{window to place this container into. If not specified a
    window is created and filled with this container.}
  \item{frame}{numeric vector of the form c(x, y, with, height)
    specifying the size and location of the container}
  \item{flags}{flags defining the behavior of this container when the
    parent container of window are resized}
}
\details{
Containers can contain any visual object in the iPlots eXtreme graphics
system. Most commonly such objects are plots, R graphics devices or
further containers. The flags of each obejct define how it will respond
to resize events. Containers automatically handle the resizing of their
children.

The model for resizing is that each object can choose to fix its borders
with respect to any of the four sides of its parent. Any sides that are
not fixed will grow proportinally. The default is to be fixed to all
four sides (or from the other point of view full expansion in both
directions) which simply fills all available space.

The currently supported flags are:
\describe{
  \item{\code{"fix.top"}}{fixed at the top}
  \item{\code{"fix.bottom"}}{fixed at the bottom}
  \item{\code{"fix.left"}}{fixed at the left margin}
  \item{\code{"fix.right"}}{fixed at the right margin}
  \item{\code{"xspring"}}{expands along the x axis (by fixing both ends) - same as \code{c("fix.left","fix.right")}}
  \item{\code{"yspring"}}{expands along the y axis (by fixing both ends) - same as \code{c("fix.top","fix.bottom")}}
  \item{\code{"xyspring"}}{expands in all directions (by attaching all
    four margins) - same as the combination of all fix attributes above}
}

The \code{window} parameter can be either \code{NULL} is which case a
new window is created and filled with the container (the size of the
window is thus defined by the size of the container) or \code{FALSE} in
which case an off-screen container is created to be attached to a window
later or a window object (of the clas \code{iWindow}).
}
\value{
Object of the class \code{iContainer}.
}
\note{Operations on the container object are by default not followed by
 an implicit redraw (as is the case for plots) to allow contruction of
 complex layouts without unnecessary redraws. Please use
 \code{\link{redraw}} after all components have been added to a
 container.
}
\seealso{
\code{\link{iplot}}, \code{\link{redraw}}
}
\examples{
# create a container (implicitly creates a new window)
a = icontainer(frame=c(0,0,400,300))

# create some random plots off-screen (window=FALSE)
x = rnorm(1000)
y = rnorm(1000)
p = iplot(x, y, window=FALSE, frame=c(0,0,200,300),
        flags=c("fix.left","fix.top","fix.bottom"))
q = iplot(y, x, window=FALSE, frame=c(200,0,200,300),
        flags=c("fix.right","fix.top","fix.bottom"))

# add the plots to the container and redraw
redraw(a + p + q)
}
\keyword{hplot}
