#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define TRIG PD2
#define ECHO PD3
#define RESET_BTN PD4

#define GREEN_LED PB0
#define YELLOW_LED PB1
#define RED_LED PB2

#define INITIAL_CAPACITY 10

uint8_t vehicles_entered = 0;
uint8_t available_slots = INITIAL_CAPACITY;
uint8_t detected = 0;

void UART_Init(unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    UCSR0B = (1 << TXEN0);

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_Transmit(char data)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_String(char *str)
{
    while (*str)
    {
        UART_Transmit(*str++);
    }
}

long getDistance()
{
    long count = 0;

    PORTD &= ~(1 << TRIG);
    _delay_us(2);

    PORTD |= (1 << TRIG);
    _delay_us(10);
    PORTD &= ~(1 << TRIG);

    while (!(PIND & (1 << ECHO)));

    while (PIND & (1 << ECHO))
    {
        count++;
        _delay_us(1);
    }

    return count / 58;
}

int main(void)
{
    char buffer[20];

    DDRB |= (1 << GREEN_LED) |
            (1 << YELLOW_LED) |
            (1 << RED_LED);

    DDRD |= (1 << TRIG);

    DDRD &= ~(1 << ECHO);

    DDRD &= ~(1 << RESET_BTN);

    PORTD |= (1 << RESET_BTN);

    UART_Init(103);

    UART_String("Parking System Started\r\n");

    while (1)
    {
        long distance = getDistance();

        UART_String("Distance = ");

        ltoa(distance, buffer, 10);
        UART_String(buffer);
        UART_String(" cm\r\n");

        if (distance < 10 &&
            distance > 0 &&
            detected == 0 &&
            vehicles_entered < INITIAL_CAPACITY)
        {
            vehicles_entered++;

            available_slots =
                INITIAL_CAPACITY - vehicles_entered;

            detected = 1;

            UART_String("Vehicle Detected\r\n");

            UART_String("Cars = ");
            itoa(vehicles_entered, buffer, 10);
            UART_String(buffer);
            UART_String("\r\n");
        }

        if (distance > 15)
        {
            detected = 0;
        }

        if (!(PIND & (1 << RESET_BTN)))
        {
            _delay_ms(20);

            if (!(PIND & (1 << RESET_BTN)))
            {
                vehicles_entered = 0;
                available_slots = INITIAL_CAPACITY;
                detected = 0;

                UART_String("SYSTEM RESET\r\n");

                while (!(PIND & (1 << RESET_BTN)));
            }
        }

        PORTB &= ~((1 << GREEN_LED) |
                   (1 << YELLOW_LED) |
                   (1 << RED_LED));

        if (available_slots > 5)
        {
            PORTB |= (1 << GREEN_LED);
            UART_String("GREEN LED ON\r\n");
        }
        else if (available_slots > 0)
        {
            PORTB |= (1 << YELLOW_LED);
            UART_String("YELLOW LED ON\r\n");
        }
        else
        {
            PORTB |= (1 << RED_LED);
            UART_String("RED LED ON - FULL\r\n");
        }

        UART_String("Available Slots = ");
        itoa(available_slots, buffer, 10);
        UART_String(buffer);
        UART_String("\r\n");

        UART_String("------------------\r\n");

        _delay_ms(500);
    }
}