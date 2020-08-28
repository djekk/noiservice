package com.server.springboot.object;

/*
 * Copyright 2001-2020 by HireRight, Inc. All rights reserved.
 * This software is the confidential and proprietary information
 * of HireRight, Inc. Use is subject to license terms.
 */

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.stereotype.Repository;


@Repository
public interface TaskRepository extends JpaRepository<Task, Integer> {
	
	@Query(value="select c from Task c where c.customer = ?1 and c.taskid = ?2")
	Task findByCustomerAndtaskId(String customer, Integer taskid);
	
	//@Query(value="select * from Task c where c.state = 'NEW' ORDER BY c.lastupdated ASC FOR UPDATE SKIP LOCKED LIMIT 1",
	@Query(value="select * from Task c WHERE c.state = 'NEW' ORDER BY c.lastupdated ASC  LIMIT 1 FOR UPDATE SKIP LOCKED", nativeQuery = true)
	Task getInquiryLock();
}
