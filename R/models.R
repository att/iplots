add.iPlot.lm <- function(x, obj, ...) {
  l <- iabline(obj, plot=x, col=4, ...)
  query(l) <- paste(c("y = ", " * x +"), format(coef(obj)[2:1], digits=3), collapse='')
  invisible(x)
}

