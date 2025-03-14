<template>
    <div class="parent">
        <p>User Actions</p>
        <button @click="router.push('/leave-room')">Leave Room</button>
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
.parent {
    display: flex;
    flex-direction: column;
}

button {
    margin-top: 40px;
    font-size: 18pt;
    color: white;
    min-width: 180px;
    min-height: 50px;
    background-color: var(--foreground);
    border: var(--vue-green) solid 3px;
    text-align: center;
    align-items: center;
    text-decoration: none;
    cursor: pointer;
}

button:hover {
    color: var(--foreground);
    background-color: var(--vue-green)
}


</style>
