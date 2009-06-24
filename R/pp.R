primitives <- function(plot)
  .Call("A_PlotPrimitives", plot)

add <- function(x, what, ...) UseMethod("add")
delete <- function(x, what, ...) UseMethod("delete")

color <- function(x, ...) UseMethod("color")
fill <- function(x, ...) UseMethod("fill")
"color<-" <- function(x, value, ...) UseMethod("color<-")
"fill<-" <- function(x, value, ...) UseMethod("fill<-")

iLine <- function(x, y) {
  o <- .Call("A_LineCreate", as.double(c(x[1],y[1],x[2],y[2])))
  class(o) <- c("iLine", "primitive")
  invisible(o)
}

iPolygon <- function(x, y) {
  o <- .Call("A_PolygonCreate", as.double(x), as.double(y));
  class(o) <- c("iPolygon", "primitive")
  invisible(o)
}

iText <- function(x, y, text) {
  if (!length(text)) stop("missing text")
  o <- .Call("A_TextCreate", as.double(c(x,y)), as.character(text))
  class(o) <- c("iText", "primitive")
  invisible(o)
}

add.default <- function(x, what, ...) {
  .Call("A_PlotAddPrimitive", x, what)
}

delete.default <- function(x, what, ...) {
  if (all(what == "all"))
    .Call("A_PlotRemoveAllPrimitives")
  else
    .Call("A_PlotRemovePrimitive", x, what)
}

`color<-.primitive` <- function(x, value, redraw=TRUE, ...) {
  value <- col2rgb(value[1], TRUE)[,1] / 256
  .Call("A_VPSetColor", x, as.double(value))
  if (redraw) .Call("A_VPRedraw", x)
  invisible(x)
}

`fill<-.primitive` <- function(x, value, redraw=TRUE, ...) {
  value <- col2rgb(value[1], TRUE)[,1] / 256
  .Call("A_VPSetFill", x, as.double(value))
  if (redraw) .Call("A_VPRedraw", x)
  invisible(x)
}

color.primitive <- function(x, ...) {
  v <- .Call("A_VPGetColor", x)
  rgb(v[1], v[2], v[3], v[4])
}

fill.primitive <- function(x, ...) {
  v <- .Call("A_VPGetFill", x)
  rgb(v[1], v[2], v[3], v[4])
}
