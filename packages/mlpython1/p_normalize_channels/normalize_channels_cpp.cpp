/*
<%
cfg['include_dirs'] = ['../mlpy']
setup_pybind11(cfg)
%>
*/
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "ndarray.h"

namespace py = pybind11;

// A custom error handler to propagate errors back to Python
class error : public std::exception {
public:
  error (const std::string& msg) : msg_(msg) {};
  virtual const char* what() const throw() {
    return msg_.c_str();
  }
private:
  std::string msg_;
};

py::array_t<float> channel_variances(py::array_t<float> X) {
	NDArray<float> Xa(X);
	if (Xa.ndim!=2) {
		printf("Incorrect number of dimensions: %ld\n",Xa.ndim);
		throw error("Incorrect number of dimensions");
	}
	int M=Xa.shape[0];
	int N=Xa.shape[1];
	if (N<2) {
		throw error("N must be at least 2");
	}
	auto X2 = X.mutable_unchecked<2>();

	auto result = py::array_t<float>(M);
	NDArray<float> Ra(result);
	for (int m=0; m<M; m++) {
		double sum=0;
		double sumsqr=0;
		for (int n=0; n<N; n++) {
			double tmp=X2(m,n);
			sum+=tmp;
			sumsqr+=tmp*tmp;
		}
		Ra.ptr[m]=(sumsqr-sum*sum/N)/(N-1);
	}
	return result;
}

void scale_channels(py::array_t<float> X, py::array_t<float> factors) {
	NDArray<float> Xa(X);
	NDArray<float> Fa(factors);
	if (Xa.ndim!=2) {
		printf("Incorrect number of dimensions: %ld\n",Xa.ndim);
		throw error("Incorrect number of dimensions");
	}
	int M=Xa.shape[0];
	int N=Xa.shape[1];

	//auto X2 = X.mutable_unchecked<2>();

	if (Fa.size!=M) {
		throw error("Incorrect size of factors");
	}

	//float *Xptr=(float *)buf_X.ptr;
	float *Fptr=(float *)Fa.ptr;
	for (int m=0; m<M; m++) {
		if (Fptr[m]!=0) {
			for (int n=0; n<N; n++) {
				//X2(m,n)/=Fptr[m];
				Xa.ptr[m+M*n]/=Fptr[m];
			}
		}
	}
}

PYBIND11_PLUGIN(normalize_channels_cpp) {
    pybind11::module m("normalize_channels_cpp", "auto-compiled c++ extension");
    m.def("channel_variances", &channel_variances, "Compute the variance of channels in a timeseries",py::arg().noconvert());
    m.def("scale_channels", &scale_channels, "Divide channels of timeseries by scale factors",py::arg().noconvert(),py::arg().noconvert());
    return m.ptr();
}
