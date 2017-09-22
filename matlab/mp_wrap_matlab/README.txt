mp_wrap_matlab -- generate mountainlab processor wrappers for matlab scripts

Setup:
cd mp_wrap_matlab
npm install fs-extra

Example usage:
cd mp_wrap_matlab/examples
../mp_wrap_matlab .

That should create a directory examples/mp_wrappers
Now copy the examples directory into mountainlab/packages and you have new processor(s)
Use mp-list-processors to verify
Note that mountainlab searches for processor specifications in .mp files
contained somewhere in mountainlab/packages
