/** @file watcher.h
 *  @brief Function prototypes the watcher task.
 *  @author Matthias Ellinger
 *  @date 13/4/2026
 */

#ifndef INC_WATCHER_H_
#define INC_WATCHER_H_

void get_stack_size(osThreadId_t thread_id[], uint32_t stack_size[], int cnt);
void print_stack_size(uint32_t stack_size[], osThreadAttr_t thread_attribute[],int cnt, UART_HandleTypeDef uart_handle);

#endif /* INC_WATCHER_H_ */
