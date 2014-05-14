#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <unistd.h>
#include <pthread.h>

#define MAX_QUEUES 10

typedef	struct	message_
{
	uint32_t	senderid;
	uint16_t	msglen;
	char		msg[0];
}message_t;


typedef	struct	queue_
{
	int			front, rear;
	int			size;
	message_t	**msgs;
}queue_t;


queue_t	*queues[MAX_QUEUES];

int
queue_init(int	size)
{
	static	int	index = -1;

	queue_t		*q = malloc(sizeof(queue_t));
	memset(q, 0, sizeof(queue_t));

	q->front = 0;
	q->rear  = -1;
	q->size  = size;
	q->msgs = malloc(sizeof(message_t) * size);
	queues[++index] = q;

	return index;

}

queue_t *
get_q_from_handle(int	index)
{
	return queues[index];
}



int
enqueue(int	q_handle, void *data)
{
	queue_t	*q = get_q_from_handle(q_handle);	
	if(q == NULL)
	{
		printf("Invalid Queue\n");
		return -1;
	}
	if(q->rear == q->size)
	{
		printf("queue is full\n");
		return -1;
	}

	q->msgs[++(q->rear)] = data;

	return 0;
}

int
is_q_empty(int q_handle)
{
	queue_t	*q = get_q_from_handle(q_handle);
	if(q == NULL)
	{
		printf("%s: Invalid queue\n", __FUNCTION__);
		return -1;
	}

	if((q->rear < 0) || (q->front > q->rear))
		return 1;

	return 0;
}

int
is_q_full(int q_handle)
{
	queue_t	*q = get_q_from_handle(q_handle);
	if(q == NULL)
    {
        printf("%s: Invalid queue\n", __FUNCTION__);
        return -1;
    }

	if(q->rear == q->size - 1)
        return 1;
	
	return 0;
}

void *
dequeue(int	q_handle)
{
	if(is_q_empty(q_handle) != 0)
		return NULL;

	queue_t		*q = get_q_from_handle(q_handle);
	message_t	*msg = q->msgs[(q->front)++];

	if(q->front > q->rear)
	{
		q->front = 0;
		q->rear = -1;
	}

	return msg;
}


void
test_q()
{
	int 		q_handle = queue_init(10);
	char		*str = "HI, This is test message";
	message_t	*msg = malloc(sizeof(message_t) + strlen(str));
	msg->senderid = getpid();
	msg->msglen = strlen(str);
	strncpy(msg->msg, str, strlen(str)); 

	enqueue(q_handle, msg);

	{
		message_t	*recv_msg = dequeue(q_handle);
		char		recv_str[1024] = "";
		strncpy(recv_str, recv_msg->msg, 1024);
		printf("+--------------------- TEST ----------------------+\n");
		printf("+Received message : %s\n", recv_str);
		printf("+--------------------- END  ----------------------+\n");
	}
}

int q_hdl; /* Queue handle for both producer and consumer */

int
producer()
{
	int			n = 5; // # of messages to send.
	message_t	*msg[5] = {NULL};
	int			ret = -1;
	char		*str[5] = { "This is First message",
							"This is Second message",
							"This is Third message",
							"This is fourth message",
							"This is Fifth Message"};

	for(int i=0; i<n; i++)
	{
		if(is_q_full(q_hdl) == 0)
		{
			msg[i] = malloc(sizeof(message_t) + strlen(str[i]));
			msg[i]->senderid = getpid();
			msg[i]->msglen = strlen(str[i]);
			strncpy(msg[i]->msg, str[i], strlen(str[i]));

			ret = enqueue(q_hdl, msg[i]);
			if(ret == 0)
				printf("Successfully enqueued %d message\n", i);
		}
		else
			printf("Failed to enqueue %d message\n", i);
	}

	printf("%s exiting\n", __FUNCTION__);
	return 0;
}

int
consumer()
{
	int			n=5;
	message_t	*msg = NULL;
	char		buf[1024] = "";

	for(int i=0; i<n; i++)
	{
		printf("Waiting for %d message\n", i);
		while((is_q_empty(q_hdl) == 0) && (i<n))
		{
			msg = dequeue(q_hdl);
			
			printf("\n+-------------- Received message count %d -------------------+\n", i);
			printf("+sender	 :	%u \n", msg->senderid);
			memset(buf, 0, sizeof(buf));
			strncpy(buf, msg->msg, sizeof(buf));
			printf("+Message :	%s\n", buf);
			printf("+-------------- END------------------------------------------+\n\n");

			i++;
		}
	}

	printf("%s : exiting...\n", __FUNCTION__);
	return 0;
}


int
main()
{
	pthread_t	tid=0;
	int			ret = 0;
	q_hdl = queue_init(3);

	ret = pthread_create(&tid, NULL, &producer, NULL);
	if(ret != 0)
	{
		perror("pthread_create");
	}
	else
		printf("pthread %d created successfully for producer\n", tid);

	sleep(5);	
	consumer();


	return 0;
}




