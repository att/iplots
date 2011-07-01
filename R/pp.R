primitives <- function(plot)
  lapply(.Call("A_PlotPrimitives", plot), function(x) { class(x) <- c("primitive", "iObject"); x })

##--- methods

color <- function(x, ...) UseMethod("color")
fill <- function(x, ...) UseMethod("fill")
"color<-" <- function(x, value, ...) UseMethod("color<-")
"fill<-" <- function(x, value, ...) UseMethod("fill<-")
hidden <- function(x, ...) UseMethod("hidden")
"hidden<-" <- function(x, value, ...) UseMethod("hidden<-")
query <- function(x, ...) UseMethod("query")
"query<-" <- function(x, value, ...) UseMethod("query<-")

ilines <- function(x, ...) UseMethod("ilines")
isegments <- function(x, ...) UseMethod("isegments")
iabline <- function(a, ...) UseMethod("iabline")
ipoints <- function(x, ...) UseMethod("ipoints")
itext <- function(x, ...) UseMethod("itext")
ipolygon <- function(x, ...) UseMethod("ipolygon")

##--- primitives constructors ---

iLine <- function(x, y, color) {
  o <- .Call("A_LineCreate", as.double(c(x[1],y[1],x[2],y[2])))
  class(o) <- c("iLine", "primitive", "iObject")
  if (!missing(color)) color(o, redraw=FALSE) <- color
  invisible(o)
}

iSegments <- function(x1, y1, x2, y2, color) {
  l <- c(length(x1), length(y1), length(x2), length(y2))
  if (!all(l == l[1])) stop("all coordinates must be of the same length")
  o <- .Call("A_SegmentsCreate", as.double(x1), as.double(y1), as.double(x2), as.double(y2))
  class(o) <- c("iSegments", "primitive", "iObject")
  if (!missing(color)) color(o, redraw=FALSE) <- color
  invisible(o)  
}

iPolygon <- function(x, y, color, fill) {
  if (length(x) != length(y)) stop("all coordinates must be of the same length")
  o <- .Call("A_PolygonCreate", as.double(x), as.double(y));
  class(o) <- c("iPolygon", "primitive", "iObject")
  if (!missing(color)) color(o, redraw=FALSE) <- color
  if (!missing(fill)) fill(o, redraw=FALSE) <- fill
  invisible(o)
}

iText <- function(x, y, text, color) {
  if (!length(text)) stop("missing text")
  o <- .Call("A_TextCreate", as.double(c(x,y)), as.character(text))
  class(o) <- c("iText", "primitive", "iObject")
  if (!missing(color)) color(o, redraw=FALSE) <- color
  invisible(o)
}

##--- add/delete

add.iPlot.primitive <- function(x, what, ...) {
  .Call("A_PlotAddPrimitive", x, what)
  redraw(x) # just to make sure for now
}

add.iPlot.pairlist <- function(x, what, ...) {
  .Call("A_PlotAddPrimitives", x, what)
  redraw(x) # just to make sure for now
}

add.primitive.primitive <- function(x, what, ...)
  CONS(x, CONS(what))

add.pairlist.primitive <- function(x, what, ...)
  CONS(what, x)

delete.iPlot.primitive <- function(x, what, ...) {
  .Call("A_PlotRemovePrimitive", x, what)
  redraw(x) # just to make sure for now
}

delete.iPlot.pairlist <- function(x, what, ...) {
  .Call("A_PlotRemovePrimitives", x, what)
  redraw(x) # just to make sure for now
}

delete.primitive.primitive <- function(x, what, ...)
  CONS(x, CONS(what))

delete.pairlist.primitive <- function(x, what, ...)
  CONS(what, x)

delete.iPlot.character <- function(x, what, ...) {
  if (all(what == "all")) {
    .Call("A_PlotRemoveAllPrimitives", x)
    redraw(x)
  } else stop("invalid argument")
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

hidden.primitive <- function(x, ...)
  .Call("A_VPGetHidden", x)

`hidden<-.primitive` <- function(x, value, ...)
  .Call("A_VPSetHidden", x, as.logical(value))

query.primitive <- function(x, ...)
  .Call("A_VPGetQuery", x)

`query<-.primitive` <- function(x, value, ...)
  .Call("A_VPSetQuery", x, value)

`$.primitive` <- function(x, name) {
  if (name == "plot") return(.po(.Call("A_VPPlot", x)))
  if (name == "color" || name == "col") return(color(x))
  if (name == "fill") return(fill(x))
  if (name == "query") return(query(x))
  if (name == "hidden") return(hidden(x))
  if (name == "context") return(.Call("A_VPGetContext", x))
  if (name == "callback" || name == "onChange") return (.Call("A_VPGetCallback", x))
  vl <- .Call("A_VPGetValue", x)
  vl[[name]]
}

`$<-.primitive` <- function(x, name, value) {
  if (name == "color" || name == "col") color(x) <- value else
  if (name == "fill") fill(x) <- value else
  if (name == "hidden") hidden(x) <- value else
  if (name == "query") query(x) <- value else
  if (name == "callback" || name == "onChange") .Call("A_VPSetCallback", x, value) else
  if (name == "context") .Call("A_VPSetContext", x, value) else
  if (name == "onSelect") .Call("A_VPSetSelCallback", x, value) else
  { vl <- .Call("A_VPGetValue", x); vl[[name]] <- value; .Call("A_VPSetValue", x, vl) }
  x
}

iabline.lm <- function(a, ...) {
  mc <- coef(a)
  if (length(mc) != 2) stop("invalid dimensions")
  iabline(mc[1], mc[2], ...)
}

iabline.default <- function(a, b, ..., plot=.Last.plot) {
  x <- plot$xlim
  y <- x * b + a
  l <- iLine(x, y, ...)
  add(plot, l)
  invisible(l)
}

ilines.default <- function(x, y, col, ..., plot=.Last.plot) {
  p <- iPolygon(x, y)
  if (!missing(col)) p$color <- col
  add(plot, p)
  invisible(p)
}

isegments.default <- function(x0, y0, x1 = x0, y1 = y0, col, ..., plot=.Last.plot) {
  p <- iSegments(x0, y0, x1, y1)
  if (!missing(col)) p$color <- col
  add(plot, p)
  invisible(p)
}

ipolygon.default <- function(x, y, border, col = NA, ..., plot=.Last.plot) {
  p <- iPolygon(x, y)
  if (!missing(border)) color(p, redraw=FALSE) <- border
  if (all(!is.na(col))) fill(p, redraw=FALSE) <- col
  add(plot, p)
  invisible(p)
}

# we need to map Ops to other functions, because Ops dispatch attempts to enforce type equality which is stupid (more precisely it doesn't displatch if it finds a function for each type even though it should have ignored the second argument)
`+.iObject` <- function(e1, e2) add(e1, e2)
`-.iObject` <- function(e1, e2) delete(e1, e2)

replacePoints <- function(p, x, y)
  .Call("A_PolygonSetPoints", p, as.double(x), as.double(y))
