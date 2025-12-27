/* la32_tlb.c */
#include <loongarch_tlb.h>
#include <memlayout.h>

// invalidate both TLB
// (clean and flush, meaning we write the data back)
void tlb_invalidate(pde_t *pgdir, uintptr_t la)
{
  tlb_invalidate_all();
}

void tlb_invalidate_all()
{
  asm volatile(".word 0x6498000");
}

uint32_t pte2tlblow(pte_t pte)
{
#ifdef LAB3_EX1
  // 提取物理地址部分并转换为 PPN 格式填入 [31:8]
  uint32_t t = (((uint32_t)pte - KERNBASE) >> 12) << 8;

  if (!ptep_present(&pte)) // 检查页表项是否存在 (PTE_P)
    return 0;

  // 设置基本属性：有效(V)，全局(G)，存储一致性(MAT)
  t |= LOONGARCH_TLB_ENTRYL_V;
  t |= LOONGARCH_TLB_ENTRYL_G;
  t |= LOONGARCH_TLB_MAT_CO;

  // 如果用户可读，设置 PLV3 (用户态权限)
  if (ptep_u_read(&pte))
  {
    t |= LOONGARCH_TLB_PLV3;
  }

  // 如果可写，设置 D (Dirty) 位
  if (ptep_s_write(&pte))
    t |= LOONGARCH_TLB_ENTRYL_D;

  return t;
#endif
}

void tlb_refill(uint32_t badaddr, pte_t *pte)
{
#ifdef LAB3_EX1
  // 1. 检查 pte 是否有效
  if (pte == NULL)
  {
    return;
  }

  // 2. LoongArch 采用双页设计，EntryLo0 对应偶数页(Bit 12 = 0)，EntryLo1 对应奇数页
  // 如果 badaddr 指向奇数页，需要将 pte 指针前移一位，指向对应的偶数页 PTE
  if (badaddr & (1 << 12))
  {
    pte--;
  }

  // 3. 获取 EntryLo0 和 EntryLo1 的值
  uint32_t tlblow0 = pte2tlblow(*pte);       // 偶数页
  uint32_t tlblow1 = pte2tlblow(*(pte + 1)); // 奇数页

  // 4. 准备 EntryHi (清除低 13 位，保留 VPN2) 并写入 TLB
  // LOONGARCH_TLB_ENTRYH_VPPN_MASK 通常是 ~0x1FFF
  tlb_replace_random(badaddr & LOONGARCH_TLB_ENTRYH_VPPN_MASK, tlblow0, tlblow1);
#endif
}
