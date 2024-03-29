package org.cscs.jprinterface.queue;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicLong;
import java.util.logging.Logger;

import org.cscs.jprinterface.lpd.PrintJob;
import org.cscs.jprinterface.queue.QueueListener.ChangeEvent;

public class DefaultQueueManager implements QueueManager {
	private static final Logger logger = Logger.getLogger(DefaultQueueManager.class.getName());
	public final Map<String, List<PrintJob>> printQueues;
	public final AtomicLong jobIdSeed;
	public final List<QueueListener> listeners;

	public DefaultQueueManager() {
		this.printQueues = Collections.synchronizedMap(new HashMap<String, List<PrintJob>>());
		
		this.listeners = new CopyOnWriteArrayList<QueueListener>();
		
		// token attempt to set the jobIds to something unique
		Calendar c = Calendar.getInstance();
		long datecode = c.get(Calendar.YEAR) * 10000 +
						(c.get(Calendar.MONTH)+1) * 100 +
						c.get(Calendar.DAY_OF_MONTH);
		
		jobIdSeed = new AtomicLong(datecode * 1000);
		
		logger.info(String.format("Starting queuemaneger with seed %d", jobIdSeed.get()));		
	}
	
	public Set<String> getQueueNames() {
		return printQueues.keySet();
	}
	
	public void createQueue(String name) {
		synchronized(printQueues) {
			if (printQueues.containsKey(name))
				throw new IllegalArgumentException("Queue already exists");
			printQueues.put(name, new LinkedList<PrintJob>());
		}
		fireQueueChange(name, QueueListener.ChangeEvent.ADD);
	}
	
	

	public List<PrintJob> getQueueContent(String name) {
		return new ArrayList<PrintJob>(printQueues.get(name));
	}
	
	public void addJob(String queueName, PrintJob job) {
		List<PrintJob> queue = printQueues.get(queueName);
		queue.add(job);		
		fireNewJob(queueName, job);		
	}
	
	void fireNewJob(String queue, PrintJob job) {
		for (QueueListener l : listeners) 
			l.jobChange(queue, job, ChangeEvent.ADD);
	}
	private void fireQueueChange(String name, ChangeEvent event) {
		for (QueueListener l : listeners) 
			l.queueChange(name, event);		
	}
	
	public long getNextJobId() {
		return jobIdSeed.incrementAndGet();
	}

	public void addListener(QueueListener listener) {
		listeners.add(listener);
	}
	
	public void removeListener(QueueListener listener) {
		listeners.remove(listener);
		
	}
}