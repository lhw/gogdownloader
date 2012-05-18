using Posix;

int main() {

	if(!Thread.supported())
		return 1;
	try {
		File f = File.new_for_path("blob");
		var ff = f.replace(null, false, FileCreateFlags.NONE);
		ff.truncate_fn(3*1024*1024);
		
		Semaphore smp = new Semaphore(0);
		var thread1_obj = new Write("blob", 0, smp);
		var thread2_obj = new Write("blob", 1024*1024, smp);
		var thread3_obj = new Write("blob", 2*1024*1024, smp);

		Thread<void*> thread1 = new Thread<void*>("thread1", thread1_obj.run);
		Thread<void*> thread2 = new Thread<void*>("thread2", thread2_obj.run);
		Thread<void*> thread3 = new Thread<void*>("thread3", thread3_obj.run);

		smp.set_queue_size(3);

		thread1.join();
		thread2.join(); 
		thread3.join(); 
	} catch(ThreadError e) {
		GLib.stderr.printf("%s\n", e.message);
		return 1;
	} catch(Error e) {
		GLib.stderr.printf("%s\n", e.message);
		return 1;
	}
	return 0;
}

class Semaphore {
	private int val;
	private Mutex mtx = Mutex();
	private Cond cv = Cond();

	public Semaphore(int val) {
		this.val = val;
	}
	public void V() {
		mtx.lock();
		val++;
		mtx.unlock();
		cv.broadcast();
	}
	public void P() {
		mtx.lock();
		while(val < 1)
			cv.wait(mtx);
		val--;
		mtx.unlock();
	}
	public void set_queue_size(int size) {
		val = size;
		cv.broadcast();
	}
}

class Write {
	private string TEST = "ABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZ";

	private string _path;
	private int64 _offset;
	private Semaphore _smp;
	private FILE _file;
	
	public Write(string path, long offset, Semaphore smp) {
		_path = path;
		_offset = offset;
		_smp = smp;

		_file = FILE.open(path, "r+");
		if(offset != 0)
			_file.seek(offset, SEEK_SET);

	}
	public void* run() {
		_smp.P();
		_file.write(TEST.data, TEST.data.length, 1);	
		_smp.V();
		return null;
	}
}
