<template>
    <div>
        <p>Admin Actions</p>
        <button @click="removeUser()">Remove User</button>
    </div>
</template>


<script setup>
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { useApiStore } from '@/store/api'
    import { useUserStore } from '@/store/user'
    import axios from 'axios'

    const router = useRouter()
    const apiStore = useApiStore()
    const userStore = useUserStore()

    const removeUser = () => {
        router.push('/remove-user')
    }
</script>


<style scoped>
button {
    font-size: 18pt;
    color: white;
    min-width: 70px;
    margin: auto;
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
