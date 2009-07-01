. <- new.env(TRUE, emptyenv())

.init.set <- function(len, name="data") {
 if (!is.null(.$len) && len == .$len) return(TRUE)
 .$len = len
 .$name = name
 .$m = .Call("A_MarkerCreate", as.integer(len))
 TRUE
}

## hack!
addCallback <- function(FUN) .Call("A_MarkerDependentCreate", .$m, FUN)

.var <- function(x, name=deparse(substitute(x))) {
 if (is.null(.$m)) .init.set(length(x))
 .Call("A_VarRegister", x, .$m, name)
}

iplot <- function(x, ...) UseMethod("iplot")
ibar <- function(x, ...) UseMethod("ibar")
ipcp <- function(x, ...) UseMethod("ipcp")
ihist <- function(x, ...) UseMethod("ihist")

redraw <- function(x, ...) UseMethod("redraw")
selected <- function(x, ...) UseMethod("selected")
select <- function(x, ...) UseMethod("select")

.mp <- function(w, p, clazz) {
  class(w) <- "iWindow"
  .Call("A_PlotSetValue", p, list(window = w, subclass = clazz))
  class(p) <- c(clazz, "iPlot")
  p
}

.po <- function(p) {
  sc <- .Call("A_PlotValue", p)$subclass
  if (is.null(sc)) sc <- character(0)
  class(p) <- c(sc, "iPlot")
  p
}

iplot.default <- function(x, y, xname=deparse(substitute(x)), yname=deparse(substitute(y)), ...) {
 vx = .var(x, xname)
 vy = .var(y, yname)
 sp = .Call("A_ScatterPlot", vx, vy, c(100,100,400,300))
 w  = .Call("A_WindowCreate", sp, c(100,100))
 .mp(w, sp, "iScatterplot")
}

ibar.factor <- function(x, xname=deparse(substitute(x)), ...) {
 vx = .var(x, xname)
 bc = .Call("A_BarPlot", vx, c(100,100,400,300))
 w  = .Call("A_WindowCreate", bc, c(100,100))
 .mp(w, bc, "iBarchart")
}

ibar.default <- function(x, ...) stop("Sorry, bar charts for this data type are not yet defined.")

ihist.default <- function(x, xname=deparse(substitute(x)), ...) {
 vx = .var(x, xname)
 h  = .Call("A_HistPlot", vx, c(100,100,400,300))
 w  = .Call("A_WindowCreate", h, c(100,100))
 .mp(w, h, "iHist")
}

ipcp.list <- function(x, ...) {
  if (length(x) < 2) stop("need at least 2 dimensions")
  v = lapply(seq.int(x), function(i) .var(x[[i]], names(x)[i]))
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

redraw.iVisual <- function(x, ...)
  invisible(.Call("A_PlotRedraw", x))

## access to virutal fields in plot objects that have pass-by-reference semantics of the whole plot object

`$.iPlot` <- function(x, name) {
  if (name == "marker") return(.Call("A_PlotPrimaryMarker", x))
  d <- .Call("A_PlotDoubleProperty", x, name)
  if (!is.null(d) && !all(is.na(d))) return(if (name %in% c("spines")) (d > 0.5) else d)
  o <- .Call("A_PlotValue", x)
  o[[name]]
}

`$<-.iPlot` <- function(x, name, value) {
  if (name %in% c("marker")) stop("read-only property")
  if (.Call("A_PlotSetDoubleProperty", x, name, value)) return(x)
  o <- .Call("A_PlotValue", x)
  o[[name]] <- value
  .Call("A_PlotSetValue", x, o)
  x
}

names.iPlot <- function(x)
  names(.Call("A_PlotValue", x))

selected.iPlot <- function(x) {
  m <- x$marker
  if (is.null(m)) stop("plot has no primary marker")
  .Call("A_MarkerSelected", m)
}

select.iPlot <- function(x, which) {
  m <- x$marker
  if (is.null(m)) stop("plot has no primary marker")
  if (!is.integer(which) && is.numeric(which)) which <- as.integer(which)
  invisible(.Call("A_MarkerSelect", m, which))
}

idev <- function(width=640, height=480, ps=12, bg=0, canvas=0, dpi=90) {
  dev <- .External("RAcinonyxDevice", width, height, ps, bg, canvas, dpi, 0L)
  w <- .Call("A_WindowCreate", dev, c(100,100))
  class(dev) <- "iVisual"
  attr(dev, "window") <- w
  dev
}
