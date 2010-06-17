add.iPlot.lm <- function(x, what, ...) {
  l <- iabline(what, plot=x, col=4, ...)
  query(l) <- paste(c("y = ", " * x +"), format(coef(what)[2:1], digits=3), collapse='')
  invisible(x)
}

