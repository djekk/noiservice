package com.server.springboot;

import com.server.springboot.object.Task;
import com.server.springboot.object.TaskService;
import com.server.springboot.states.TaskStateMachine;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.bind.annotation.RequestMapping;

import java.sql.Timestamp;
import java.util.*;

import org.springframework.beans.factory.annotation.*;
import org.springframework.http.*;

import org.springframework.web.bind.annotation.*;

/*
 * Copyright 2001-2020 by HireRight, Inc. All rights reserved.
 * This software is the confidential and proprietary information
 * of HireRight, Inc. Use is subject to license terms.
 */

/*
1 .install postgre

2. create table

DROP TABLE task;
CREATE TABLE task (
  id Integer CONSTRAINT task_pk PRIMARY KEY,
  customer VARCHAR(20) NOT NULL,
  taskid Integer NOT NULL,
  inquiry VARCHAR(500) NOT NULL,
  result VARCHAR(500),
  owner VARCHAR(100),
  state VARCHAR(20) NOT NULL,
  lastupdated TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
DROP INDEX IF EXISTS task_ct_idx;
CREATE INDEX task_ct_idx ON task (customer, taskid);
DROP SEQUENCE task_id_seq;
CREATE SEQUENCE task_id_seq START 1;

insert into task (id, customer, taskid, inquiry, state) values (nextval('task_id_seq'), 'customer1', 1, 'dfg', 'NEW');
select * from task;

3. create noiadmin
		
		
		CREATE ROLE noiadmin WITH
		LOGIN
		SUPERUSER
		INHERIT
		CREATEDB
		CREATEROLE
		NOREPLICATION
		ENCRYPTED PASSWORD 'SCRAM-SHA-256$4096:1p4djwLVoqVhdTgc16WjFw==$F691KAQtY2DE0c7eJKGD6YRBGUdXjIJA2Gw5RFI8o9Y=:izMQYMvLVVM4ikFBBi8qNwLcy1t+Z1HqI+S+V0K4f8o=';
*/
//postgresql
//pgAdmin on windows
//http://127.0.0.1:61110/browser/

//https://app.diagrams.net/
//https://github.com/lankydan/spring-boot-post-tutorial/blob/master/spring-boot-rest-tutorial/src/main/java/lankydan/tutorial/springboot/PersonRestController.java
//https://www.codejava.net/frameworks/spring-boot/spring-boot-restful-crud-api-examples-with-mysql-database

//https://blog.netgloo.com/2014/10/27/using-mysql-in-spring-boot-via-spring-data-jpa-and-hibernate/
// depl https://www.hetzner.com/

//https://www.guru99.com/postgresql-create-database.html

 /*

 
 //https://www.baeldung.com/exception-handling-for-rest-with-spring/
//https://stackabuse.com/monitoring-spring-boot-apps-with-micrometer-prometheus-and-grafana/
//		https://www.hetzner.com/

/*
* select * from task;

--delete from task;
SELECT version();

SELECT id
  FROM task
  WHERE state = 'NEW'
  ORDER BY lastupdated ASC
  FOR UPDATE SKIP LOCKED
  LIMIT 1;
  
 select * from Task c
  ORDER BY c.lastupdated ASC LIMIT 1 FOR UPDATE SKIP LOCKED;
  
  
  update task set result = '11fdjkljhklhjkdf' where id = 1;
    commit;
 

* 157.167.49.180:9999
* */

@RestController
public class TaskController
{
	@Autowired
	private TaskService service;
	
	@RequestMapping("/")
	public String index() {
		return "Hi hacker!";
	}
	// RESTful API methods for Retrieval operations
	
	@GetMapping("/tasks")
	public List<Task> list() {
		return service.listAll();
	}
	
	@GetMapping("/task/{id}")
	public ResponseEntity<Task> get(@PathVariable Integer id) {
		try {
			Task task = service.get(id);
			return new ResponseEntity<Task>(task, HttpStatus.OK);
		} catch (NoSuchElementException e) {
			return new ResponseEntity<Task>(HttpStatus.NOT_FOUND);
		}
	}
	/*
	public Task getByCustomerAndTaskId(String customer, Integer taskid)
	{
		List<Task> tasks = list();
		for( Task task : tasks)
		{
			if(task.getCustomer().equals(customer) &&
					task.getTaskId().equals(taskid))
			{
				return task;
			}
		}
		
		return null;
	}
	*/
	
	// RESTful API method for Create operation
	
	/*
	 * Eduard kidaet v bazu rabotu
		http://localhost:8080/setInquiry  POST
		Body zaporsa: {"customer":"jfghfjghhj","taskid":2,"inquiry":"xfgxfgxfgfgdfgd"}
	 */
	@PostMapping("/setInquiry")
	public ResponseEntity<Task> setInquiry(@RequestBody Task task) {
		
		try {
			Task findTask = service.getByCustomerAndTaskId( task.getCustomer(), task.getTaskId());
			if(findTask != null)
			{
				task.setId(findTask.getId());
			}
			task.setState(TaskStateMachine.NEW);
			task.setLastUpdated(new Timestamp(System.currentTimeMillis()));
			task.setResult(null);
			task.setOwner(null);
			service.save(task);
			return new ResponseEntity<Task>(task, HttpStatus.OK);
		} catch (NoSuchElementException e) {
			return new ResponseEntity<Task>(HttpStatus.NOT_FOUND);
		}
	}
	
	/*
	 c++ zabiraet rabotu s server
	POST
	http://localhost:8080/getInquiry
	Body zaporosa: {"owner":"owner"}   otsilaet 4to 
	 */
	
	// RESTful API method for read operation
	@PostMapping("/getInquiry")
	public ResponseEntity<Task> getInquiry(@RequestBody Task intask) {
		try {
			if(intask != null && intask.getOwner() != null)
			{
				Task task = service.getInquiryLock(intask.getOwner());
				return new ResponseEntity<Task>(task, HttpStatus.OK);
			}
			return new ResponseEntity<Task>(HttpStatus.NOT_FOUND);
			
		} catch (NoSuchElementException e) {
			return new ResponseEntity<Task>(HttpStatus.NOT_FOUND);
		}
		
	}
	
	/*
	c++ kidaet proshitanij result
	http://localhost:8080/setResult
	PUT
	Body zaporosa: {"id":3, "result":"hgj"}
	*/
	// RESTful API method for Update operation
	@PutMapping("/setResult")
	public ResponseEntity<Task> setResult(@RequestBody Task task) {
		try {
			Task existTask = service.get(task.getId());
			existTask.setResult(task.getResult());
			existTask.setState(TaskStateMachine.READY);
			existTask.setLastUpdated(new Timestamp(System.currentTimeMillis()));
			service.save(existTask);
			return new ResponseEntity<Task>(existTask, HttpStatus.OK);
		} catch (NoSuchElementException e) {
			return new ResponseEntity<Task>(HttpStatus.NOT_FOUND);
		}
	}
	/*
	Eduard poluchaet tak result s bazi dannih sdelanij c++
	http://localhost:8080/getResult
	POST
	Body zaporsa:{"customer":"customer1","taskid":1}
	*/
	@PostMapping("/getResult")
	public ResponseEntity<Task> getResult(@RequestBody Task task) {
		try {
			Task findTask = service.getByCustomerAndTaskId( task.getCustomer(), task.getTaskId());
			if(findTask!= null &&
					TaskStateMachine.isReady(findTask.getState()))
			{
				return new ResponseEntity<Task>(findTask, HttpStatus.OK);
			}
		} catch (NoSuchElementException e) {
			return new ResponseEntity<Task>(HttpStatus.NOT_FOUND);
		}
		
		return new ResponseEntity<Task>(HttpStatus.NOT_FOUND);
	}
}
