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
    error(const std::string& msg)
        : msg_(msg){};
    virtual const char* what() const throw()
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

void extract_clips(py::array_t<float> &clips_out,py::array_t<float> X, py::array_t<int> times, int clip_size)
{
    NDArray<float> Xa(X);
    NDArray<float> Ca(clips_out);
    NDArray<int> Ta(times);
    if (Xa.ndim != 2) {
        printf ("Incorrect number of dimensions: %ld\n", Xa.ndim);
        throw error("Incorrect number of dimensions");
    }
    int L = Ta.size;
    int M = Xa.shape[0];
    int N = Xa.shape[1];
    int T = clip_size;

    if ((Ca.shape[0]!=M)||(Ca.shape[1]!=T)||(Ca.shape[2]!=L)) {
        printf ("%ld,%ld,%ld  <>  %d,%d,%d",Ca.shape[0],Ca.shape[1],Ca.shape[2],M,T,L);
        throw error("Incorrect size of clips_out");
    }

    int Tmid=(int)((T+1)/2-1);
    for (int i=0; i<L; i++) {
        int t=Ta.ptr[i];
        if ((0<=t-Tmid)&&(t-Tmid+T-1<N)) {
            int ii=M*(t-Tmid);
            int jj=M*T*i;
            for (int m=0; m<M; m++) {
                for (int a=0; a<T; a++) {
                    Ca.ptr[jj]=Xa.ptr[ii];
                    ii++;
                    jj++;
                }
            }
        }
    }
}

PYBIND11_PLUGIN(extract_clips_cpp)
{
    pybind11::module m("extract_clips_cpp", "");
    m.def("extract_clips", &extract_clips, "", py::arg().noconvert(),py::arg(),py::arg(),py::arg());
    return m.ptr();
}
