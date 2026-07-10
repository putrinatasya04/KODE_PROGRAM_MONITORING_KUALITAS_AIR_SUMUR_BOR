#define TURBIDITY_PIN 34

float calibrateTurbidityFromAdc(int adc)
{
    float voltage =
    adc*(3.3/4095.0);

    float NTU =
    (-73.53*voltage)+164;

    if(NTU<0)
        NTU=0;

    return NTU;
}

void setup()
{
    Serial.begin(115200);

    analogReadResolution(12);

    analogSetPinAttenuation(
    TURBIDITY_PIN,
    ADC_11db
    );
}

void loop()
{
    long total=0;

    for(int i=0;i<10;i++)
    {
        total += analogRead(
        TURBIDITY_PIN);

        delay(20);
    }

    int adc=total/10;

    float voltage=
    adc*(3.3/4095.0);

    float NTU=
    calibrateTurbidityFromAdc(adc);

    String status;

    if(NTU<=5)
        status="JERNIH";

    else if(NTU<=25)
        status="AGAK KERUH";

    else
        status="KERUH";

    Serial.print(
    "ADC: ");

    Serial.print(adc);

    Serial.print(
    " | Volt: ");

    Serial.print(voltage,3);

    Serial.print(
    " V | NTU: ");

    Serial.print(NTU,2);

    Serial.print(
    " | Status: ");

    Serial.println(status);

    delay(500);
}