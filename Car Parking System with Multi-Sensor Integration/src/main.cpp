#include <avr/io.h>
#include <util/delay.h>

long getDistance()
{
    long count = 0;

    // Trigger pulse
    PORTD &= ~(1 << PD2);
    _delay_us(2);
    PORTD |= (1 << PD2);
    _delay_us(10);
    PORTD &= ~(1 << PD2);

    // Wait for echo to go HIGH
    while (!(PIND & (1 << PD3)));

    // Measure HIGH time
    while (PIND & (1 << PD3))
    {
        count++;
        _delay_us(1);
    }

    // Convert to approximate distance (cm)
    return count / 58;
}

int main(void)
{
    // LEDs as outputs
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

    // TRIG as output
    DDRD |= (1 << PD2);

    // ECHO and Reset Button as inputs
    DDRD &= ~((1 << PD3) | (1 << PD4));

    // Enable pull-up for reset button
    PORTD |= (1 << PD4);

    int totalSlots = 10;
    int cars = 0;
    int detected = 0;

    while (1)
    {
        long distance = getDistance();

        // Count one car when object comes within 10 cm
        if (distance < 10 && detected == 0 && cars < totalSlots)
        {
            cars++;
            detected = 1;
        }

        // Ready to detect the next car
        if (distance > 15)
        {
            detected = 0;
        }

        // Reset button
        if (!(PIND & (1 << PD4)))
        {
            cars = 0;
            detected = 0;
            _delay_ms(300);
        }

        // Turn off all LEDs
        PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2));

        // LED status
        if ((totalSlots - cars) > 5)
            PORTB |= (1 << PB0);      // Green
        else if ((totalSlots - cars) > 0)
            PORTB |= (1 << PB1);      // Yellow
        else
            PORTB |= (1 << PB2);      // Red

        _delay_ms(100);
    }
}