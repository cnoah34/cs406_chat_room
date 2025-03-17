<template>
    <div style="display: flex; flex-direction: column;">
        <p>Admin Actions</p>
        <button @click="router.push('/remove-user')" class="button-gray action">Remove User</button>
    </div>
</template>


<script setup>
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { useApiStore } from '@/store/api'

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
