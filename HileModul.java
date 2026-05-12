import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class HileModul {
    public static boolean reachEnabled = false;
    public static boolean espEnabled = false;
    public static double reachDistance = 3.5;

    public static void main(String[] args) {
        // Windows görünümü ayarla
        try { UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName()); } catch (Exception e) {}

        JFrame frame = new JFrame("SO Optimizer v1.0");
        frame.setSize(300, 250);
        frame.setLayout(new GridLayout(4, 1));

        // Reach Kontrolü
        JCheckBox reachBox = new JCheckBox("Reach (Uzak Vurus)");
        JSlider reachSlider = new JSlider(30, 60, 35); // 3.0 - 6.0 arası
        
        // ESP Kontrolü
        JCheckBox espBox = new JCheckBox("ESP (Oyuncuyu Gor)");

        // Buton ve Etki
        JButton applyBtn = new JButton("Ayarları Uygula");
        applyBtn.addActionListener(e -> {
            reachEnabled = reachBox.isSelected();
            espEnabled = espBox.isSelected();
            reachDistance = reachSlider.getValue() / 10.0;
            JOptionPane.showMessageDialog(frame, "Ayarlar Güncellendi!");
        });

        frame.add(reachBox);
        frame.add(new JLabel(" Reach Mesafesi:"));
        frame.add(reachSlider);
        frame.add(espBox);
        frame.add(applyBtn);

        frame.setAlwaysOnTop(true);
        frame.setVisible(true);
        
        // ESP Döngüsü (Arka planda çalışır)
        startESPLoop();
    }

    public static void startESPLoop() {
        new Thread(() -> {
            while(true) {
                if(espEnabled) {
                    // Burada Minecraft'ın render sınıflarına erişilip 
                    // oyuncuların etrafına kutu çizme komutları gönderilir.
                }
                try { Thread.sleep(50); } catch (InterruptedException e) {}
            }
        }).start();
    }
}

