. <- new.env(TRUE, emptyenv())

.init.set <- function(len, name="data") {
 if (!is.null(.$len) && len == .$len) return(TRUE)
 .$len = len
 .$name = name
 .$m = .Call("A_MarkerCreate", as.integer(len))
 TRUE
}

.var <- function(x) {
 if (is.null(.$m)) .init.set(length(x))
 .Call("A_VarRegister", x, .$m)
}

iplot <- function(x, ...) UseMethod("iplot")
ibar <- function(x, ...) UseMethod("ibar")
ipcp <- function(x, ...) UseMethod("ipcp")
ihist <- function(x, ...) UseMethod("ihist")

redraw <- function(x, ...) UseMethod("redraw")

.mp <- function(w, p, clazz) {
  class(w) <- "iWindow"
  .Call("A_PlotSetValue", p, list(window = w))
  class(p) <- c(clazz, "iPlot")
  p
}

iplot.default <- function(x, y, ...) {
 vx = .var(x)
 vy = .var(y)
 sp = .Call("A_ScatterPlot", vx, vy, c(100,100,400,300))
 w  = .Call("A_WindowCreate", sp, c(100,100))
 .mp(w, sp, "iScatterplot")
}

ibar.factor <- function(x, ...) {
 vx = .var(x)
 bc = .Call("A_BarPlot", vx, c(100,100,400,300))
 w  = .Call("A_WindowCreate", bc, c(100,100))
 .mp(w, bc, "iBarchart")
}

ibar.default <- function(x, ...) stop("Sorry, bar charts for this data type are not yet defined.")

ihist.default <- function(x, ...) {
 vx = .var(x)
 h  = .Call("A_HistPlot", vx, c(100,100,400,300))
 w  = .Call("A_WindowCreate", h, c(100,100))
 .mp(w, h, "iHist")
}

ipcp.list <- function(x, ...) {
  if (length(x) < 2) stop("need at least 2 dimensions")
  v = lapply(x, .var)
  p = .Call("A_PCPPlot", v, c(100,100,400,300))
  w  = .Call("A_WindowCreate", p, c(100,100))
  .mp(w, p, "iPCP")
}

ipcp.data.frame <- ipcp.list

ipcp.default <- function(x, ...) {
  n = length(x)
  l = unlist(lapply(list(...),function(q) length(q)==n))
  l = c(list(x),list(...)[l])
  ipcp.list(l)
}

redraw.iPlot <- function(x, ...)
  invisible(.Call("A_PlotRedraw", x))

## access to virutal fields in plot objects that have pass-by-reference semantics of the whole plot object

`$.iPlot` <- function(x, name) {
  o <- .Call("A_PlotValue", x)
  o[[name]]
}

`$<-.iPlot` <- function(x, name, value) {
  o <- .Call("A_PlotValue", x)
  o[[name]] <- value
  .Call("A_PlotSetValue", x, o)
  x
}

names.iPlot <- function(x)
  names(.Call("A_PlotValue", x))

