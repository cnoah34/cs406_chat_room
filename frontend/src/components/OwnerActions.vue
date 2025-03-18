<template>
    <div style="display: flex; flex-direction: column;">
        <button @click="router.push('/promote-user')" class="button-gray action">Promote User</button>
        <button @click="router.push('/demote-admin')" class="button-gray action">Demote Admin</button>
        <button @click="router.push('/delete-room')" class="button-gray action">Delete Room</button>
    </div>
</template>


<script setup>
    import { useRouter } from 'vue-router'
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
