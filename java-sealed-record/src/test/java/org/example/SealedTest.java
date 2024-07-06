package org.example;

import org.junit.jupiter.api.Test;

public class SealedTest {
    // 1. sealed class A permits B,C,D => only allow B,C,D inherit from A
    // 2. B,C,D should be marked with
    //    1. sealed: only allow some classes inherit from B/C/D
    //    2. final: no inheritance
    //    3. non-sealed: like other normal class, can be inherited by any classes
    @Test
    public void testHero() {
        Hero h = new TankHero();
        System.out.println(h);
    }
}

// 坦克英雄的抽象
non-sealed class TankHero extends Hero {
}

// 输出英雄的抽象
non-sealed class AttackHero extends Hero {
}

// 辅助英雄的抽象
non-sealed class SupportHero extends Hero {

}

// 坦克英雄：阿利斯塔
class Alistar extends TankHero {

}

// 输出英雄：伊泽瑞尔
class Ezreal extends AttackHero {

}

// 辅助英雄：索拉卡
class Soraka extends SupportHero {

}

sealed class Hero permits TankHero, AttackHero, SupportHero {

}