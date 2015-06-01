import java.util.Scanner;
import java.io.IOException;

public class Default {
public static void main(String[] args) throws IOException {
Scanner input = new Scanner(System.in);
byte[] array = new byte[30000];
int pointer = 0;
array[pointer]++;
while (array[pointer] == 1){ 
pointer++;
array[pointer] = (byte)System.in.read();
System.out.print((char)array[pointer]);
pointer--;
}
}
}
