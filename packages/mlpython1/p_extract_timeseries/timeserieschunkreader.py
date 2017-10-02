from mlpy import DiskReadMda

class TimeseriesChunkInfo:
	def __init__(self):
		self.i1=0
		self.i2=0
		self.t1=0
		self.t2=0
		self.size=0

class TimeseriesChunkReader:
	def __init__(self, chunk_size=1000, overlap_size=0, t1=-1, t2=-1):
		self._chunk_size=chunk_size
		self._overlap_size=overlap_size
		self._t1=t1
		self._t2=t2
	def run(self, mdafile_path, func):
		X=DiskReadMda(mdafile_path)
		M,N = X.N1(),X.N2()
		if self._t1<0:
			self._t1=0
		if self._t2<0:
			self._t2=N-1
		t=self._t1
		while t <= self._t2:
			t1=t
			t2=min(self._t2,t+self._chunk_size-1)
			s1=max(0,t1-self._overlap_size)
			s2=min(N-1,t2+self._overlap_size)
			chunk=X.readChunk(i1=0, N1=M, i2=s1, N2=s2-s1+1)
			info=TimeseriesChunkInfo()
			info.t1=t1
			info.t2=t2
			info.i1=t1-s1
			info.i2=t2-s1
			info.size=t2-t1+1
			if not func(chunk, info):
				return False
			t=t+self._chunk_size
		return True

