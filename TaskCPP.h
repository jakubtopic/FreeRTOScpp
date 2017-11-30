/**
 * @file TaskCPP.h
 * @brief FreeRTOS Task Wrapper
 *
 * This file contains a set of lightweight wrappers for tasks using FreeRTOS
 *
 * @copyright (c) 2007-2015 Richard Damon
 * @author Richard Damon <richard.damon@gmail.com>
 * @parblock
 * MIT License:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * It is requested (but not required by license) that any bugs found or
 * improvements made be shared, preferably to the author.
 * @endparblock
 *
 * @ingroup FreeRTOSCpp
 *
 * @defgroup FreeRTOSCpp Free RTOS C++ Wrapper
 */
#ifndef TaskCPP_H
#define TaskCPP_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Names for Base set of Priorities.
 *
 * Assigns used based names to priority levels, optimized for configMAX_PRIORITIES = 6 for maximal distinctions
 * with reasonable colapsing for smaller values. If configMAX_PRIORITIES is >6 then some values won't have names here, but
 * values could be created and cast to the enum type.
 *
 * | configMAX_PRIORITIES: | 1 | 2 | 3 | 4 | 5 | 6 | N>6 | Use                                                |
 * | --------------------: | - | - | - | - | - | - | :-: | :------------------------------------------------- |
 * | TaskPrio_Idle         | 0 | 0 | 0 | 0 | 0 | 0 |  0  | Non-Real Time operations, Tasks that don't block   |
 * | TaskPrio_Low          | 0 | 1 | 1 | 1 | 1 | 1 |  1  | Non-Critical operations                            |
 * | TaskPrio_HMI          | 0 | 1 | 1 | 1 | 1 | 2 |  2  | Normal User Interface                              |
 * | TaskPrio_Mid          | 0 | 1 | 1 | 2 | 2 | 3 | N/2 | Semi-Critical, Deadlines, not much processing      |
 * | TaskPrio_High         | 0 | 1 | 2 | 3 | 3 | 4 | N-2 | Urgent, Short Deadlines, not much processing       |
 * | TaskPrio_Highest      | 0 | 1 | 2 | 3 | 4 | 5 | N-1 | Critical, do NOW, must be quick (Used by FreeRTOS) |
 *
 *
 * @ingroup FreeRTOSCpp
 */
enum TaskPriority {
	TaskPrio_Idle = 0,													///< Non-Real Time operations. tasks that don't block
	TaskPrio_Low = ((configMAX_PRIORITIES)>1),                         	///< Non-Critical operations
	TaskPrio_HMI = (TaskPrio_Low + ((configMAX_PRIORITIES)>5)),			///< Normal User Interface Level
	TaskPrio_Mid = ((configMAX_PRIORITIES)/2),							///< Semi-Critical, have deadlines, not a lot of processing
	TaskPrio_High = ((configMAX_PRIORITIES)-1-((configMAX_PRIORITIES)>4)), ///< Urgent tasks, short deadlines, not much processing
	TaskPrio_Highest = ((configMAX_PRIORITIES)-1) 						///< Critical Tasks, Do NOW, must be quick (Used by FreeRTOS)
};

/**
 * @brief Lowest Level Wrapper.
 *
 * Create the specified task with a provided task function.
 *
 * If the Task object is destroyed, the class will be deleted (if deletion has been enabled)
 *
 * Note, many of the member functions of Task are available conditionally
 * based on the corresponding options being turned on in FreeRTOSConfig.h
 *
 * Example Usage:
 * @code
 * void taskfun(void *parm) {
 *     while(1) {
 *         vTaskDelay(1);
 *     }
 * }
 *
 * Task taskTCB("Task", taskfun, TaskPrio_Low, configMINIMAL_STACK_SIZE);
 * @endcode
 *
 * Note, I, personally, create virtually all of my tasks with a global/file scope
 * object. I try to set up my systems to not be creating and destroying tasks as the
 * system runs.
 *
 * One warning, do NOT create tasks as local (automatic) variables in main(),
 * as FreeRTOS in some ports reuses the stack from main as the interrupt stack
 * when the scheduler starts.
 *
 * @ingroup FreeRTOSCpp
 * @todo Look at adding member functions for task manipulation
 * @todo Look at adding TaskNotify operations
 * @todo Support static allocation added in FreeRTOS V9.
 *
 */
class Task {
public:
  /**
   * @brief Constructor.
   *
   * @param name The name of the task.
   * @param taskfun The function implementing the task, should have type void (*taskfun)(void *)
   * @param priority The priority of the task. Use the TaskPriority enum values or a related value converted to a TaskPriority
   * @param stackDepth Size of the stack to give to the task
   * @param parm the parameter passed to taskFun. Defaults to NULL.
   *
   * Upon construction the task will be created. See the FreeRTOS function
   * xTaskCreate() for more details on the parameters
   *
   */
  Task(char const*name, void (*taskfun)(void *), TaskPriority priority,
       unsigned portSHORT stackDepth, void * parm = 0) {
    xTaskCreate(taskfun, name, stackDepth, parm, priority, &handle);
  }
  /**
   * @brief Destructor.
   *
   * If deletion is enabled, delete the task.
   */
  virtual ~Task() {
#if INCLUDE_vTaskDelete
    if(handle){
      vTaskDelete(handle);
    }
#endif
    return;
  }

  /**
   * @brief Get Task Handle.
   * @return the task handle.
   */
  TaskHandle_t getHandle() const { return handle; }

#if INCLUDE_vTaskPriorityGet
  /**
   * @brief Get Task priority
   *
   * Only available if INCLUDE_vTaskPriorityGet == 1
   * @return The priority of the Task.
   */
  TaskPriority priority() const { return static_cast<TaskPriority>(uxTaskPriorityGet(handle); }
#endif

#if INCLUDE_vTaskPrioritySet
  /**
   * @brief Set Task priority
   *
   * Only available if INCLUDE_vTaskPrioritySet == 1
   * @param priority The TaskPriority to give the Task.
   */
  void priority(TaskPriority priority) { vTaskPrioritySet(handle, priority); }
#endif

#if INCLUDE_vTaskSuspend
  /**
   * @brief Suspend the Task.
   *
   * Only available if INCLUDE_vTaskSuspend == 1
   */
  void suspend() { vTaskSuspend(handle); }

  /**
   * @brief Resume the Task.
   *
   * Only available if INCLUDE_vTaskSuspend == 1
   */
  void resume() { vTaskResume(handle); }

# if INCLUDE_vTaskResumeFromISR
  /**
   * @brief Resume task from ISR.
   *
   * Note: Only functions with _ISR should be used inside Interupt service routines.
   *
   * Only available if INCLUDE_vTaskSuspend == 1 and INCLUDE_vTaskResumeFromISR == 1
   * @returns True if ISR should request a context switch.
   */
  bool resume_ISR() { return xTaskResumeFromISR(handle); }
# endif

#endif

protected:
  TaskHandle_t handle;  ///< Handle for the task we are managing.
private:
#if __cplusplus < 201101L
    Task(Task const&);      ///< We are not copyable.
    void operator =(Task const&);  ///< We are not assignable.
#else
    Task(Task const&) = delete;      ///< We are not copyable.
    void operator =(Task const&) = delete;  ///< We are not assignable.
#endif // __cplusplus

};

/**
 * @brief Make a class based task.
 * Derive from TaskClass and the 'task()' member function will get called as the task based on the class.
 *
 * If task() returns the task will be deleted if deletion has been enabled.
 *
 * Example Usage:
 * @code
 *
 * class MyClass : public TaskClass {
 * public:
 *   MyClass() :
 *     TaskClass("MyClass", TaskPrio_Low, configMINIMAL_STACK_SIZE) // Note, these parameters could come from parameters of the constructor
 *     {
 *     }
 *
 *   virtual void task() {
 *     // Do something
 *   }
 * };
 *
 * MyClass myclass; // Create object (and tas)
 * @endcode
 * @ingroup FreeRTOSCpp
 */
class TaskClass : public Task {
public:
  /**
   * @brief Constructor
   *
   * @param name The name of the task.
   * @param priority The priority of the task. Use the TaskPriority enum values or a related value converted to a TaskPriority
   * @param stackDepth Size of the stack to give to the task
   *
   * Note: At construction time the task will be created, so if the the scheduler has been started, the created task needs to
   * have a priority less than the creating task so it can't start until after the class deriving from TaskClass has finished
   * it constructor (or other measures need to have been taken to make sure this happens, like stopping the scheduler).
   *
   * Also, class based tasks don't get a void* parameter, instead they have the object
   * of which they are a member.
   *
   */
  TaskClass(char const*name,TaskPriority priority,
           unsigned portSHORT stackDepth) :
    Task(name, &taskfun, priority, stackDepth, this)
  {
  }
  /**
   * @brief task function.
   * The Class derived from TaskClass implements it task function as the task() member function.
   */
  virtual void task() = 0;

private:
  /**
   * Trampoline for task.
   *
   * @todo Note, this is a static function so normally compatible by calling convention
   * with an ordinary C function, like FreeRTOS expects. For maximum portablity
   * we can change this to a free function declared as extern "C"
   */
  static void taskfun(void* parm) {
    static_cast<TaskClass *>(parm) -> task();
    // If we get here, task has returned, delete ourselves or block indefinitely.
#if INCLUDE_vTaskDelete
    static_cast<TaskClass *>(parm)->handle = 0;
    vTaskDelete(0); // Delete ourselves
#else
    while(1)
      vTaskDelay(portMAX_DELAY);
#endif
  }
};

#endif
