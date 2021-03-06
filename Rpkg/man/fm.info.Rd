% Generated by roxygen2 (4.1.1): do not edit by hand
% Please edit documentation in R/flashmatrix.R
\name{fm.info}
\alias{fm.info}
\alias{fm.is.sparse}
\alias{fm.is.sym}
\alias{fm.is.vector}
\alias{fm.matrix.layout}
\title{The information of a FlashMatrix object}
\usage{
fm.is.sym(fm)

fm.matrix.layout(fm)

fm.is.sparse(fm)

fm.is.vector(fm)
}
\arguments{
\item{fm}{The FlashMatrix object}
}
\value{
`fm.is.sym' and `fm.is.sparse' returns boolean constants.
}
\description{
Functions for providing the basic information of a matrix.
}
\details{
`fm.is.sym' indicates whether a matrix is symmetric.

`fm.matrix.layout' indicates how data in a matrix is organized.
`fm.is.sparse' indicates whether a matrix is sparse.

`fm.is.vector' indicates whether a FlashMatrix object is a vector.
}
\author{
Da Zheng <dzheng5@jhu.edu>
}

