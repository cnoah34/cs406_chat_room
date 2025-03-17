<template>
    <div style="display: flex; flex-direction: column;">
        <p>User Actions</p>
        <button @click="router.push('/leave-room')" class="button-gray action">Leave Room</button>
    </div>
</template>


<script setup>
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { useApiStore } from '@/store/api'
    import { useUserStore } from '@/store/user'
    import axios from 'axios'

    const router = useRouter()
</script>


<style scoped>
.action {
    margin-top: 40px;
    font-size: 18pt;
    min-width: 180px;
    min-height: 50px;
}

</style>
