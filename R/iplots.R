.ipe <- new.env(TRUE, emptyenv())

.init.set <- function(len, name="data") {
 if (!is.null(.ipe$len) && len == .ipe$len) return(TRUE)
 .ipe$len = len
 .ipe$name = name
 .ipe$m = .Call("A_MarkerCreate", as.integer(len))
 TRUE
}

## hack!
addCallback <- function(FUN) .Call("A_MarkerDependentCreate", .ipe$m, FUN)
reset <- function() rm(len,name,m,envir=.ipe)
restore <- function() { if (exists(".last.ipe")).ipe <<- .last.ipe; invisible(.ipe$len) }

.var <- function(x, name=deparse(substitute(x))) {
 if (is.null(.ipe$m)) .init.set(length(x))
 if (.ipe$len != length(x)) { .last.ipe <<- .ipe; reset() }
 .Call("A_VarRegister", x, .ipe$m, name)
}

iplot <- function(x, ...) UseMethod("iplot")
ibar <- function(x, ...) UseMethod("ibar")
ipcp <- function(x, ...) UseMethod("ipcp")
ihist <- function(x, ...) UseMethod("ihist")

redraw <- function(x, ...) UseMethod("redraw")
selected <- function(x, ...) UseMethod("selected")
select <- function(x, ...) UseMethod("select")
move <- function(x, ...) UseMethod("move")

add <- function(x, ...) UseMethod("add")
add.iPlot <- function(x, obj, ...) UseMethod("add.iPlot", obj)
add.iContainer <- function(x, obj, ...) UseMethod("add.iContainer", obj)
add.primitive <- function(x, obj, ...) UseMethod("add.primitive", obj)
add.pairlist <- function(x, obj, ...) UseMethod("add.pairlist", obj)
add.default <- function(x, obj, ...) UseMethod("add.default", obj)

delete <- function(x, ...) UseMethod("delete")
delete.iPlot <- function(x, obj, ...) UseMethod("delete.iPlot", obj)

add.iContainer.iVisual <- function(x, obj, ...) {
  invisible(.Call("A_ContainerAdd", x, obj))
}

.po <- function(p) {
  sc <- .Call("A_PlotValue", p)$subclass
  if (is.null(sc)) sc <- character(0)
  class(p) <- c(sc, "iPlot", "iVisual", "iObject")
  p
}

.flag.names <- c("fix.top", "fix.left", "fix.bottom", "fix.right", "fix.width", "fix.height", "xspring", "yspring", "xyspring")
.flag.values <- c(0x100L, 0x200L, 0x400L, 0x800L, 0x1000L, 0x2000L, 0xa00L, 0x500L, 0xf00L)

.flags <- function(x) {
  f <- match(x, .flag.names)
  if (any(is.na(f))) stop("invalid flag: ", paste(x[is.na(f)], collapse=", "))
  if (!length(f)) 0L else as.integer(sum(.flag.values[f]))
}

.default.window.placement <- function(frame)
  c(100, 100)

.do.plot <- function(callName, className, window, frame, flags, ...) {
  flags <- if (missing(flags)) 0xf00L else .flags(flags)
  if (missing(window)) window <- NULL
  if (missing(frame)) frame <- c(0, 0, 400, 300)
  p <- .Call(callName, ..., frame, flags)
  if (is.null(window)) {
    window <- .Call("A_WindowCreate", p, .default.window.placement(frame))
    class(window) <- "iWindow"
  }
  if (!className %in% "iContainer") {
    .Call("A_PlotSetValue", p, list(window = window, subclass = className))
    class(p) <- c(className, "iPlot", "iVisual", "iObject")
    .Last.plot <<- p
    if (.Platform$OS.type == "windows") redraw(p, TRUE)
  } else class(p) <- c(className, "iVisual", "iObject")
  p
}

iplot.default <- function(x, y, xname=deparse(substitute(x)), yname=deparse(substitute(y)), ..., window, frame, flags) {
  if (!is.character(xname) || length(xname) != 1) stop("invalid xname argument - must be a character vector of length one")
  if (!is.character(yname) || length(yname) != 1) stop("invalid yname argument - must be a character vector of length one")
 vx = .var(x, xname)
 vy = .var(y, yname)
 .do.plot("A_ScatterPlot", "iScatterplot", window, frame, flags, vx, vy)
}

ibar.factor <- function(x, xname=deparse(substitute(x)), ..., window, frame, flags) {
  if (!is.character(xname) || length(xname) != 1) stop("invalid xname argument - must be a character vector of length one")
  vx = .var(x, xname)
  .do.plot("A_BarPlot", "iBarchart", window, frame, flags, vx)
}

ibar.default <- function(x, ...) stop("Sorry, bar charts for this data type are not yet defined.")

ihist.default <- function(x, xname=deparse(substitute(x)), ..., window, frame, flags) {
  if (!is.character(xname) || length(xname) != 1) stop("invalid xname argument - must be a character vector of length one")
  vx = .var(x, xname)
  .do.plot("A_HistPlot", "iHist", window, frame, flags, vx)
}

ipcp.list <- function(x, ..., window, frame, flags) {
  if (length(x) < 2) stop("need at least 2 dimensions")
  v = lapply(seq.int(x), function(i) .var(x[[i]], names(x)[i]))
  .do.plot("A_PCPPlot", "iPCP", window, frame, flags, v)
}

ipcp.data.frame <- ipcp.list

ipcp.default <- function(x, ..., window, frame, flags) {
  if (!is.vector(x)) stop("invalid variable")
  n = length(x)
  l = unlist(lapply(list(...),function(q) length(q) == n))
  l = c(list(x),list(...)[l])
  ipcp.list(l)
}

icontainer <- function(parent = NULL, window, frame, flags)
  .do.plot("A_ContainerCreate", "iContainer", window, frame, flags, parent)

redraw.iPlot <- function(x, entirely=FALSE, ...)
  invisible(.Call("A_PlotRedraw", x, entirely))

redraw.iVisual <- function(x, ...)
  invisible(.Call("A_PlotRedraw", x))

## access to virutal fields in plot objects that have pass-by-reference semantics of the whole plot object

`$.iPlot` <- function(x, name) {
  if (name == "marker") return(.Call("A_PlotPrimaryMarker", x))
  if (name == "xlim")
    return(c(x$xlim.low, x$xlim.hi))
  if (name == "ylim")
    return(c(x$ylim.low, x$ylim.hi))
  if (name == "frame")
    return(.Call("A_PlotGetFrame", x))
  d <- .Call("A_PlotDoubleProperty", x, name)
  if (!is.null(d) && !all(is.na(d))) return(if (name %in% c("spines")) (d > 0.5) else d)
  o <- .Call("A_PlotValue", x)
  o[[name]]
}

`$<-.iPlot` <- function(x, name, value) {
  if (name %in% c("marker")) stop("read-only property")
  if (name %in% c("xlim","ylim")) {
    if (!is.numeric(value) || length(value) != 2)
      stop("invalid range specification - must be a numeric vector of length two")
    .Call("A_PlotSetDoubleProperty", x, paste(name,".low",sep=''), value[1])
    .Call("A_PlotSetDoubleProperty", x, paste(name,".hi",sep=''), value[2])
    return(x)
  }
  if (name == "frame") {
    if (!is.numeric(value) || length(value) >4)
      stop("invalid frame specification")
    if (length(value) < 4) {
      cf <- .Call("A_PlotGetFrame", x)
      cf[1:length(value)] <- value
      value <- cf
    }
    .Call("A_PlotSetFrame", x, as.double(value))
    return(x)
  }
  if (.Call("A_PlotSetDoubleProperty", x, name, value)) return(x)
  o <- .Call("A_PlotValue", x)
  o[[name]] <- value
  .Call("A_PlotSetValue", x, o)
  x
}

move.iVisual <- function(x, xpos, ypos, redraw=TRUE) {
  f <- .Call("A_VisualGetFrame", x)
  if (!missing(xpos)) f[1] <- as.double(xpos)[1]
  if (!missing(ypos)) f[2] <- as.double(ypos)[1]
  .Call("A_VisualSetFrame", x, f)
  if (redraw) redraw(x)
  invisible(x)
}

move.iWindow <- function(x, xpos, ypos) {
  if (missing(xpos) || !is.numeric(xpos) || !length(xpos) == 1 ||
      missing(ypos) || !is.numeric(ypos) || !length(ypos) == 1)
    stop("invalid window position specification")
  .Call("A_WindowMoveAndResize", x, c(xpos, ypos), NULL)
  invisible(x)
}

resize.iVisual <- function(x, width, height, redraw=TRUE) {
  f <- .Call("A_VisualGetFrame", x)
  if (!missing(width)) f[3] <- as.double(width)[1]
  if (!missing(height)) f[4] <- as.double(height)[1]
  .Call("A_VisualSetFrame", x, f)
  if (redraw) redraw(x)
  invisible(x)
}

resize.iWindow <- function(x, width, height) {
  if (missing(width) || !is.numeric(width) || !length(width) == 1 ||
      missing(height) || !is.numeric(height) || !length(height) == 1)
    stop("invalid window size specification")
  .Call("A_WindowMoveAndResize", x, NULL, c(width, height))
  invisible(x)
}

names.iPlot <- function(x)
  c(names(.Call("A_PlotValue", x)), "marker")

iset.selected <- function() {
  if (is.null(.ipe$m)) stop("no active iSet");
  .Call("A_MarkerSelected", m)
}  

iset.select <- function(what) {
  if (is.null(.ipe$m)) stop("no active iSet");
  if (!is.integer(which) && is.numeric(which)) which <- as.integer(which)
  invisible(.Call("A_MarkerSelect", m, which))
}

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

idev <- function(width=640, height=480, ps=12, bg=0, canvas=0, dpi=90, window, flags) {
  flags <- if (missing(flags)) 0L else .flags(flags)
  dev <- .External("RAcinonyxDevice", width, height, ps, bg, canvas, dpi, flags)
  if (missing(window)) {
    window <- .Call("A_WindowCreate", dev, .default.window.placement())
    attr(dev, "window") <- window
  }
  class(dev) <- c("iVisual", "iObject")
  dev
}

### tools

CONS <- function(head, tail=NULL) .Call("A_CONS", head, tail)

print.iObject <- function(x, ...) { cat(.Call("A_Describe", x),"\n"); x }
print.primitive <- function(x, ...) { cat("iPlot primitive",.Call("A_Describe", x),"\n"); x }
print.iPlot <- function(x, ...) { cat(.Call("A_PlotCaption", x), " (", .Call("A_Describe", x), ")\n", sep=''); x }
print.iWindow <- function(x, ...) { cat("iPlots window", .Call("A_Describe", x), "\n"); x }
