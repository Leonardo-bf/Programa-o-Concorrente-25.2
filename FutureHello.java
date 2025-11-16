package Atividade3;
/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Exemplo de uso de futures */
/* -------------------------------------------------------------------*/
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import java.util.ArrayList;
import java.util.List;

class PrimoCallable implements Callable<Boolean> {
  private final int numero;

  public PrimoCallable(int numero) {
    this.numero = numero;
  }

  private boolean ehPrimo(int n) {
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (int i = 3; i * i <= n; i += 2) {
      if (n % i == 0) return false;
    }
    return true;
  }

  @Override
  public Boolean call() {
    return ehPrimo(numero);
  }
}

public class FutureHello {
  private static final int NTHREADS = 10;

  public static void main(String[] args) {
    final int N = 100000; 
    ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
    List<Future<Boolean>> resultados = new ArrayList<>();

    for (int i = 1; i <= N; i++) {
      Callable<Boolean> tarefa = new PrimoCallable(i);
      Future<Boolean> future = executor.submit(tarefa);
      resultados.add(future);
    }

    int totalPrimos = 0;
    for (Future<Boolean> future : resultados) {
      try {
        if (future.get()) totalPrimos++;
      } catch (InterruptedException | ExecutionException e) {
        e.printStackTrace();
      }
    }

    executor.shutdown();
    System.out.println("Quantidade de números primos entre 1 e " + N + ": " + totalPrimos);
  }
}
