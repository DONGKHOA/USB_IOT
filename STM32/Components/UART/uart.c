///*
// * uart.c
// *
// *  Created on: 24-June-2024
// *      Author: DongKhoa
// */
//
///*********************
// *      INCLUDES
// *********************/
//
//#include "uart.h"
//
///**********************
// *      TYPEDEFS
// **********************/
//
//typedef struct
//{
//    int front, rear, size;
//
//    uint8_t *queue_arr;
//} queueHandle_t;
//
///******************************
// *  STATIC PROTOTYPE FUNCTION
// ******************************/
//
//static void QUEUE_Init(queueHandle_t *queue_x, uint32_t size);
//static uint8_t QUEUE_Is_Empty(queueHandle_t *queue_x);
//static uint8_t QUEUE_Is_Full(queueHandle_t *queue_x);
//static void QUEUE_Push_Data(queueHandle_t * queue_x, uint8_t element);
//static uint8_t QUEUE_Pull_Data(queueHandle_t * queue_x);
//
///**********************
// *  STATIC VARIABLES
// **********************/
//
//static uint8_t          rxData;
//static queueHandle_t    rxBuffer;
//
//static uint8_t          txData;
//static queueHandle_t    txBuffer;
//
///*********************
// *   GLOBAL FUNCTION
// *********************/
//
///**
// * The function `UARTConfig` initializes UART buffers and enables the UART Data Register not empty
// * Interrupt.
// *
// * @param uart The `uart` parameter is a pointer to a structure representing a Universal Asynchronous
// * Receiver/Transmitter (UART) peripheral in the microcontroller.
// * @param uart_irqn The `uart_irqn` parameter is the interrupt number associated with the UART
// * peripheral. It is used to configure the interrupt handling for the UART peripheral in the
// * microcontroller.
// * @param enable_echo The `enable_echo` parameter is a boolean value that determines whether echo
// * functionality should be enabled for the UART communication. If `enable_echo` is set to `true`, the
// * UART will echo back received characters to the sender. If it is set to `false`, no echo
// * functionality will be enabled.
// */
//void UARTConfig(USART_TypeDef * uart, IRQn_Type uart_irqn, bool enable_echo)
//{
//	QUEUE_Init(&rxBuffer, UART_RX_BUFFER_SIZE);
//	QUEUE_Init(&txBuffer, UART_TX_BUFFER_SIZE);
//
//	/* Enable the UART Data Register not empty Interrupt */
////	LL_USART_EnableIT_RXNE(uart);
//}
//
//void UARTWrite(USART_TypeDef *uart, int c)
//{
//
//}
//
//void UARTSendstring(USART_TypeDef *uart, const char *s)
//{
//
//}
//
///********************
// *  STATIC FUNCTION
// ********************/
//
///**
// * The function `QUEUE_Init` initializes a queue with the specified size and allocates memory for the
// * queue array.
// *
// * @param queue_x The `queue_x` parameter is a pointer to a structure of type `queueHandle_t`.
// * @param size The `size` parameter specifies the maximum number of elements that the queue can hold.
// */
//static void QUEUE_Init(queueHandle_t *queue_x, uint32_t size)
//{
//    queue_x->front = -1;
//    queue_x->rear = - 1;
//    queue_x->size = size;
//    queue_x->queue_arr = (uint8_t *)malloc(size * sizeof(uint8_t));
//}
//
///**
// * The function `QUEUE_Is_Empty` checks if a queue is empty based on the front index.
// *
// * @param queue_x The `queue_x` parameter is a pointer to a `queueHandle_t` structure, which likely
// * represents a queue data structure in the code.
// *
// * @return The function `QUEUE_Is_Empty` is returning a value of type `uint8_t`, which is an unsigned
// * 8-bit integer. The return value is the result of the comparison `(queue_x->front == -1)`, which
// * checks if the front index of the queue pointed to by `queue_x` is equal to -1. If the front index is
// * -1, the function will
// */
//static uint8_t QUEUE_Is_Empty(queueHandle_t *queue_x)
//{
//    return (queue_x->front == -1);
//}
//
///**
// * The function `QUEUE_Is_Full` checks if a queue is full based on the rear index and the queue size.
// *
// * @param queue_x The `queue_x` parameter is a pointer to a structure or object of type
// * `queueHandle_t`, which likely represents a queue data structure.
// *
// * @return The function `QUEUE_Is_Full` is returning a boolean value indicating whether the queue is
// * full or not. It returns 1 if the queue is full (rear is at the last index of the queue), and 0 if
// * the queue is not full.
// */
//static uint8_t QUEUE_Is_Full(queueHandle_t *queue_x)
//{
//    return (queue_x->rear == (queue_x->size - 1));
//}
//
///**
// * The function `QUEUE_Push_Data` adds an element to a queue if it is not full.
// *
// * @param queue_x queue_x is a pointer to a structure representing a queue. The structure contains
// * members such as front (indicating the front index of the queue), rear (indicating the rear index of
// * the queue), and queue_arr (an array to store elements in the queue).
// * @param element The `element` parameter in the `QUEUE_Push_Data` function represents the data element
// * that you want to add to the queue. It is of type `uint8_t`, which means it is an unsigned 8-bit
// * integer value that can range from 0 to 255. When you call
// *
// * @return If the `QUEUE_Is_Full(queue_x)` function returns true, the function `QUEUE_Push_Data` will
// * return without pushing the element into the queue.
// */
//static void QUEUE_Push_Data(queueHandle_t * queue_x, uint8_t element)
//{
//    if (QUEUE_Is_Full(queue_x))
//    {
//        return;
//    }
//    if (queue_x->front == -1)
//    {
//        queue_x->front = 0;
//    }
//
//    queue_x->rear = queue_x->rear + 1;
//    queue_x->queue_arr[queue_x->rear] = element;
//}
//
///**
// * The function `QUEUE_Pull_Data` pulls data from a queue and updates the front and rear pointers
// * accordingly.
// *
// * @param queue_x The `queue_x` parameter is a pointer to a structure of type `queueHandle_t`, which
// * likely contains information about a queue such as the queue array, front index, and rear index.
// *
// * @return The function `QUEUE_Pull_Data` returns the item that is pulled from the queue. If the queue
// * is empty, it returns `0xff`.
// */
//static uint8_t QUEUE_Pull_Data(queueHandle_t * queue_x)
//{
//    if (QUEUE_Is_Empty(queue_x))
//    {
//        return 0xff;
//    }
//    uint8_t item = queue_x->queue_arr[queue_x->front];
//    queue_x->front = queue_x->front + 1;
//    if (queue_x->front > queue_x->rear)
//    {
//        queue_x->front = -1;
//        queue_x->rear = -1;
//    }
//
//    return item;
//}
//
///*********************
// * INTERRUPT FUNCTION
// *********************/
//
//void UART_ISR (USART_TypeDef *uart)
//{
//
//}
