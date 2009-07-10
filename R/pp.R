primitives <- function(plot)
  lapply(.Call("A_PlotPrimitives", plot), function(x) { class(x) <- "primitive"; x })

##--- methods

color <- function(x, ...) UseMethod("color")
fill <- function(x, ...) UseMethod("fill")
"color<-" <- function(x, value, ...) UseMethod("color<-")
"fill<-" <- function(x, value, ...) UseMethod("fill<-")

ilines <- function(x, ...) UseMethod("ilines")
iabline <- function(a, ...) UseMethod("iabline")
ipoints <- function(x, ...) UseMethod("ipoints")
itext <- function(x, ...) UseMethod("itext")
ipolygon <- function(x, ...) UseMethod("ipolygon")

##--- primitives constructors ---

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

##--- add/delete

add.iPlot.primitive <- function(x, what, ...) {
  .Call("A_PlotAddPrimitive", x, what)
  redraw(x) # just to make sure for now
}

delete.iPlot.primitive <- function(x, what, ...) {
  .Call("A_PlotRemovePrimitive", x, what)
  redraw(x) # just to make sure for now
}

delete.iPlot.character <- function(x, what, ...) {
  if (all(what == "all"))
    .Call("A_PlotRemoveAllPrimitives")
  else stop("invalid argument")
}


##--- primitive properties

`color<-.primitive` <- function(x, value, redraw=TRUE, ...) {
  value <- col2rgb(value[1], TRUE)[,1] / 255
  .Call("A_VPSetColor", x, as.double(value))
  if (redraw) .Call("A_VPRedraw", x)
  invisible(x)
}

`fill<-.primitive` <- function(x, value, redraw=TRUE, ...) {
  value <- col2rgb(value[1], TRUE)[,1] / 255
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

`$.primitive` <- function(x, name) {
  if (name == "plot") return(.po(.Call("A_VPPlot", x)))
  if (name == "color") return(color(x))
  if (name == "fill") return(fill(x))
  if (name == "callback") return (.Call("A_VPGetCallback", x))
  NULL
}

`$<-.primitive` <- function(x, name, value) {
  if (name == "color") color(x) <- value else
  if (name == "fill") fill(x) <- value else
  if (name == "callback") .Call("A_VPSetCallback", x, value) else
  stop("no writable property", name)
  x
}

iabline.lm <- function(a, ...) {
  mc <- coef(a)
  if (length(mc) != 2) stop("invalid dimensions")
  iabline(mc[1], mc[2], ...)
}

iabline.default <- function(a, b, ...) {
  
}

ilines.default <- function(x, y, col, plot, ...) {
  p <- iPolygon(x, y)
  if (!missing(col)) p$color <- col
  if (missing(plot) && exists(".Last.plot")) plot <- .Last.plot
  add(plot, p)
  p
}


replacePoints <- function(p, x, y)
  .Call("A_PolygonSetPoints", p, as.double(x), as.double(y))
